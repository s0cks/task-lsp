package lsp

import (
	"context"
	"encoding/json"
	"fmt"

	"taskfile-lsp/internal/document"
	"taskfile-lsp/internal/rpc"
)

func (s *Server) handleCodeAction(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p CodeActionParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}

	actions := []CodeAction{}

	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok {
		return actions, nil
	}

	actions = append(actions, RemoveIncompleteNodeCodeAction(doc, p.TextDocument.URI, s.ws, &p)...)

	task, found := doc.parsed.FindTaskAt(document.Pos{
		Row: p.Range.Start.Line,
		Col: p.Range.Start.Character,
	})

	if found {
		actions = append(actions, CodeAction{
			Title: fmt.Sprintf("Test %s", task.Name()),
			Kind:  "quickfix",
			Diagnostics: []Diagnostic{
				{
					Range:   Range{},
					Code:    "task",
					Message: "",
				},
			},
			Edit: &WorkspaceEdit{
				Changes: map[string][]TextEdit{},
			},
		})
	}

	return actions, nil
}
