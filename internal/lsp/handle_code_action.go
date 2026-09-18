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

	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok {
		return actions, nil
	}

	actions = append(actions, RemoveIncompleteNodeCodeAction(doc, p.TextDocument.URI, s.ws, &p)...)

	return actions, nil
}
