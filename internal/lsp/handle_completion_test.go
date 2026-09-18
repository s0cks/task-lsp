package lsp

import (
	"context"
	"encoding/json"
	"testing"

	"taskfile-lsp/internal/workspace"
)

func completionAt(t *testing.T, s *Server, uri string, pos Position) []CompletionItem {
	t.Helper()

	params, err := json.Marshal(CompletionParams{
		TextDocumentPositionParams: TextDocumentPositionParams{
			TextDocument: TextDocumentIdentifier{URI: uri},
			Position:     pos,
		},
	})
	if err != nil {
		t.Fatalf("marshal: %v", err)
	}

	result, rpcErr := s.handleCompletion(context.Background(), nil, params)
	if rpcErr != nil {
		t.Fatalf("handleCompletion: %v", rpcErr)
	}

	list, ok := result.(CompletionList)
	if !ok {
		t.Fatalf("unexpected result type %T", result)
	}

	return list.Items
}

func hasLabel(items []CompletionItem, label string) bool {
	for _, it := range items {
		if it.Label == label {
			return true
		}
	}

	return false
}

func TestCompletion_LocalTaskInDeps(t *testing.T) {
	const uri = "file:///Taskfile.yaml"
	text := "tasks:\n  build:\n    deps:\n      - \n  test:\n    cmds:\n      - echo test\n"

	s := newTestServer()
	openDoc(t, s, uri, text)
	defer s.docs.Close(uri)

	items := completionAt(t, s, uri, Position{Line: 3, Character: 8})

	if !hasLabel(items, "test") {
		t.Errorf("expected local task \"test\" to be suggested; got %+v", items)
	}

	if hasLabel(items, "build") {
		t.Logf("note: build (the task being edited) is also suggested -- not wrong, just not filtered out")
	}
}

func TestCompletion_CrossFileNamespacedTaskInDeps(t *testing.T) {
	dir := t.TempDir()
	rootPath := dir + "/Taskfile.yml"
	docsPath := dir + "/docs/Taskfile.yml"

	writeTestFile(t, rootPath, "includes:\n  docs:\n    taskfile: ./docs/Taskfile.yml\ntasks:\n  build:\n    deps:\n      - \n")
	writeTestFile(t, docsPath, "tasks:\n  generate:\n    cmds:\n      - echo generate\n")

	s := newTestServer()
	s.loadWorkspaceRoot(dir)

	uri := workspace.PathToURI(rootPath)

	items := completionAt(t, s, uri, Position{Line: 6, Character: 8})

	if !hasLabel(items, "docs:generate") {
		t.Errorf("expected cross-file task \"docs:generate\" to be suggested; got %+v", items)
	}
}

func TestCompletion_TemplateVar(t *testing.T) {
	const uri = "file:///Taskfile.yaml"
	text := "vars:\n  GREETING: hello\ntasks:\n  build:\n    cmds:\n      - echo {{.\n"

	s := newTestServer()
	openDoc(t, s, uri, text)
	defer s.docs.Close(uri)

	items := completionAt(t, s, uri, Position{Line: 5, Character: 16})

	if !hasLabel(items, "GREETING") {
		t.Errorf("expected local var \"GREETING\" to be suggested; got %+v", items)
	}

	if !hasLabel(items, "TASK") {
		t.Errorf("expected builtin \"TASK\" to be suggested; got %+v", items)
	}
}

func TestCompletion_TaskBodyKeywords(t *testing.T) {
	const uri = "file:///Taskfile.yaml"
	text := "tasks:\n  build:\n    \n"

	s := newTestServer()
	openDoc(t, s, uri, text)
	defer s.docs.Close(uri)

	items := completionAt(t, s, uri, Position{Line: 2, Character: 4})

	if !hasLabel(items, "cmds") {
		t.Errorf("expected task-body keyword \"cmds\"; got %+v", items)
	}

	if hasLabel(items, "GREETING") {
		t.Errorf("did not expect var names in task-body keyword completion; got %+v", items)
	}
}
