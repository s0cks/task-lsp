package lsp

import (
	"context"
	"encoding/json"
	"taskfile-lsp/internal/rpc"
	"taskfile-lsp/internal/taskfile"
)

type CodeActionsGenerator func(doc *Document, file *taskfile.File, params *CodeActionParams) ([]CodeAction, error)

var codeActionGenerators = []CodeActionsGenerator{
	CreateUndefinedTaskCodeAction,
	GenRefactorTaskActions,
}

func (s *Server) handleCodeAction(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p CodeActionParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}

	actions := []CodeAction{}
	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok || doc.Parsed == nil {
		return actions, nil
	}

	for _, gen := range codeActionGenerators {
		new_actions, err := gen(doc, doc.Parsed, &p)
		if err != nil {
			return nil, rpc.NewErrorf(rpc.InternalError, "failed to generate code actions: %v", err)
		}

		actions = append(actions, new_actions...)
	}

	return actions, nil
}
