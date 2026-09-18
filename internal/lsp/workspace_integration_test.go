package lsp

import (
	"bytes"
	"context"
	"encoding/json"
	"io"
	"os"
	"path/filepath"
	"testing"

	"taskfile-lsp/internal/rpc"
	"taskfile-lsp/internal/workspace"
)

func writeTestFile(t *testing.T, path, content string) {
	t.Helper()
	if err := os.MkdirAll(filepath.Dir(path), 0o755); err != nil {
		t.Fatalf("mkdir: %v", err)
	}

	if err := os.WriteFile(path, []byte(content), 0o644); err != nil {
		t.Fatalf("write: %v", err)
	}
}

func TestWorkspaceInitialize_CrossFileDiagnosticsThroughRealHandlers(t *testing.T) {
	dir := t.TempDir()

	rootPath := filepath.Join(dir, "Taskfile.yml")
	docsPath := filepath.Join(dir, "docs", "Taskfile.yml")

	writeTestFile(t, rootPath, `
includes:
  docs:
    taskfile: ./docs/Taskfile.yml
tasks:
  build:
    deps: [docs:generate, docs:nonexistent]
    cmds:
      - echo build
`)

	writeTestFile(t, docsPath, `
tasks:
  generate:
    cmds:
      - echo generate
`)

	s := newTestServer()
	conn := rpc.NewConn(bytes.NewReader(nil), io.Discard)

	rootURI := workspace.PathToURI(rootPath)
	initParams, _ := json.Marshal(InitializeParams{RootURI: &rootURI})
	if _, rpcErr := s.handleInitialize(context.Background(), conn, initParams); rpcErr != nil {
		t.Fatalf("handleInitialize: %v", rpcErr)
	}

	rootText, err := os.ReadFile(rootPath)
	if err != nil {
		t.Fatalf("read root: %v", err)
	}

	openParams, _ := json.Marshal(DidOpenTextDocumentParams{
		TextDocument: TextDocumentItem{
			URI:        rootURI,
			LanguageID: "yaml",
			Version:    1,
			Text:       string(rootText),
		},
	})
	s.handleDidOpen(context.Background(), conn, openParams)

	diags := s.diagCache[rootURI]

	foundValid := false
	foundBroken := false
	for _, d := range diags {
		switch d.Message {
		case `task "build" depends on undefined task "docs:generate"`:
			foundValid = true

		case `task "build" depends on undefined task "docs:nonexistent"`:
			foundBroken = true

		}
	}

	if foundValid {
		t.Errorf("docs:generate is a real cross-file task (loaded from disk via includes on initialize) and should not be flagged; diags: %+v", diags)
	}

	if !foundBroken {
		t.Errorf("docs:nonexistent should be flagged as a missing cross-file reference; diags: %+v", diags)
	}
}
