package lsp

import (
	"context"
	"encoding/json"
	"taskfile-lsp/internal/rpc"
)

func (s *Server) handleCodeAction(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p CodeActionParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}

	actions := []CodeAction{}

	//TODO(@s0cks): implement
	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok || doc.parsed == nil {
		actions = append(actions, CodeAction{
			Title:       "Test",
			Kind:        "refactor",
			Diagnostics: []Diagnostic{},
			Edit: &WorkspaceEdit{
				Changes: map[string][]TextEdit{},
			},
		})
		return actions, nil
	}

	// filter := NewDiagnosticCodePredicate("missing-reference")
	// p.VisitDiagnosticsMatching(doc, filter, func(name string, diagnostic *Diagnostic) bool {
	// 	actions = append(actions, CodeAction{
	// 		Title:       "Remove reference",
	// 		Kind:        "refactor",
	// 		Diagnostics: []Diagnostic{*diagnostic},
	// 		Edit: &WorkspaceEdit{
	// 			Changes: map[string][]TextEdit{},
	// 		},
	// 	})
	// 	return true
	// })

	return actions, nil
}
