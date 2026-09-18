package workspace

import (
	"os"
	"path/filepath"
	"testing"

	"taskfile-lsp/internal/analysis"
)

func writeFile(t *testing.T, path, content string) {
	t.Helper()
	if err := os.MkdirAll(filepath.Dir(path), 0o755); err != nil {
		t.Fatalf("mkdir: %v", err)
	}
	if err := os.WriteFile(path, []byte(content), 0o644); err != nil {
		t.Fatalf("write: %v", err)
	}
}

func setupWorkspace(t *testing.T) (*Workspace, string, string) {
	t.Helper()
	dir := t.TempDir()

	rootPath := filepath.Join(dir, "Taskfile.yml")
	docsPath := filepath.Join(dir, "docs", "Taskfile.yml")

	writeFile(t, rootPath, `
includes:
  docs:
    taskfile: ./docs/Taskfile.yml
tasks:
  build:
    deps: [docs:generate, docs:nonexistent, missing-task]
    cmds:
      - echo build
  a:
    deps: [docs:b]
    cmds:
      - echo a
`)

	writeFile(t, docsPath, `
includes:
  root:
    taskfile: ../Taskfile.yml
tasks:
  generate:
    cmds:
      - echo generate
  b:
    deps: [root:a]
    cmds:
      - echo b
`)

	ws := New()
	rootURI := PathToURI(rootPath)
	rootText, err := os.ReadFile(rootPath)
	if err != nil {
		t.Fatalf("read root: %v", err)
	}
	ws.Load(rootURI, 1, string(rootText), true)

	return ws, rootURI, PathToURI(docsPath)
}

func TestCrossFileMissingReferences(t *testing.T) {
	ws, rootURI, _ := setupWorkspace(t)

	f, ok := ws.File(rootURI)
	if !ok || f.Parsed == nil {
		t.Fatal("root file not loaded")
	}

	diags := analysis.RunAll(rootURI, f.Parsed, ws, []analysis.Pass{analysis.MissingReferencesPass{}})

	got := map[string]bool{}
	for _, d := range diags {
		got[d.Message] = true
	}

	if got[`task "build" depends on undefined task "docs:generate"`] {
		t.Error("docs:generate is a real cross-file task and should resolve, but was flagged missing")
	}
	if !got[`task "build" depends on undefined task "docs:nonexistent"`] {
		t.Errorf("docs:nonexistent should be flagged as missing across files; got: %+v", got)
	}
	if !got[`task "build" depends on undefined task "missing-task"`] {
		t.Errorf("missing-task should be flagged as missing locally; got: %+v", got)
	}
}

func TestCrossFileCyclicDependency(t *testing.T) {
	ws, rootURI, _ := setupWorkspace(t)

	f, ok := ws.File(rootURI)
	if !ok || f.Parsed == nil {
		t.Fatal("root file not loaded")
	}

	diags := analysis.RunAll(rootURI, f.Parsed, ws, []analysis.Pass{analysis.CyclicDepsPass{}})

	if len(diags) == 0 {
		t.Fatal("expected a cross-file cycle (a -> docs:b -> root:a) to be detected, got none")
	}

	found := false
	for _, d := range diags {
		t.Logf("cycle diagnostic: %s", d.Message)
		if d.Code == analysis.CodeCyclicDependency {
			found = true
		}
	}
	if !found {
		t.Errorf("no cyclic-dependency diagnostic in %+v", diags)
	}
}

func TestIncludeCycleDetected(t *testing.T) {
	ws, _, _ := setupWorkspace(t)

	cycles := ws.DetectIncludeCycles()
	if len(cycles) == 0 {
		t.Fatal("expected an include cycle (root includes docs, docs includes root)")
	}
	for _, c := range cycles {
		t.Logf("include cycle: %s (at %s)", c.Message, c.URI)
	}
}

func TestChangePropagatesToIncludingFile(t *testing.T) {
	ws, rootURI, docsURI := setupWorkspace(t)

	f, ok := ws.File(docsURI)
	if !ok {
		t.Fatal("docs file not tracked")
	}

	affected := ws.Load(docsURI, 2, `
tasks:
  generate:
    cmds:
      - echo generate
`, false)

	foundRoot := false
	foundDocs := false
	for _, u := range affected {
		if u == rootURI {
			foundRoot = true
		}
		if u == docsURI {
			foundDocs = true
		}
	}

	if !foundDocs {
		t.Error("the edited file itself should be in the affected set")
	}
	if !foundRoot {
		t.Errorf("root includes docs, so editing docs (which removed task b, breaking root's docs:b dep) should mark root affected too; got %+v", affected)
	}

	rootFile, _ := ws.File(rootURI)
	diags := analysis.RunAll(rootURI, rootFile.Parsed, ws, []analysis.Pass{analysis.MissingReferencesPass{}})
	foundBrokenDep := false
	for _, d := range diags {
		if d.Message == `task "a" depends on undefined task "docs:b"` {
			foundBrokenDep = true
		}
	}
	if !foundBrokenDep {
		t.Errorf("removing docs:b should make root's task \"a\" -> docs:b dep newly broken; diags: %+v", diags)
	}

	_ = f
}
