package lsp

import (
	"bytes"
	"context"
	"encoding/json"
	"strings"
	"testing"

	"taskfile-lsp/internal/rpc"
)

func TestPublish_NeverEmitsNullDiagnostics(t *testing.T) {
	var out bytes.Buffer
	conn := rpc.NewConn(bytes.NewReader(nil), &out)

	s := newTestServer()
	s.publish(conn, "file:///Taskfile.yaml", 1, nil)

	if strings.Contains(out.String(), `"diagnostics":null`) {
		t.Fatalf("publish emitted a null diagnostics array, which crashes clients (e.g. Neovim's vim.json.decode -> vim.NIL, then #diags fails):\n%s", out.String())
	}

	if !strings.Contains(out.String(), `"diagnostics":[]`) {
		t.Fatalf("expected an empty array for nil diagnostics, got:\n%s", out.String())
	}
}

func TestHandleDidChange_CleanFileNeverPublishesNullDiagnostics(t *testing.T) {
	var out bytes.Buffer
	conn := rpc.NewConn(bytes.NewReader(nil), &out)

	s := newTestServer()

	openParams, err := json.Marshal(DidOpenTextDocumentParams{
		TextDocument: TextDocumentItem{
			URI:        "file:///Taskfile.yaml",
			LanguageID: "yaml",
			Version:    1,
			Text:       "tasks:\n  build:\n    cmds:\n      - echo build\n",
		},
	})

	if err != nil {
		t.Fatalf("marshal: %v", err)
	}

	s.handleDidOpen(context.TODO(), conn, openParams)

	if strings.Contains(out.String(), `"diagnostics":null`) {
		t.Fatalf("didOpen on a clean file published null diagnostics:\n%s", out.String())
	}
}
