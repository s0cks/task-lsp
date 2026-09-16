package lsp

import (
	"context"
	"encoding/json"
	"taskfile-lsp/internal/rpc"
)

func (s *Server) handleDefinition(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p TextDocumentPositionParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}

	//TODO(@s0cks): implement
	//
	// doc, ok := s.docs.Get(p.TextDocument.URI)
	// if !ok || doc.Parsed == nil {
	// 	return nil, nil
	// }
	//
	// name, _, ok := doc.Parsed.NameOrRefAt(toTFPos(p.Position))
	// if !ok {
	// 	return nil, nil
	// }
	//
	// task, ok := doc.Parsed.Tasks[name]
	// if !ok {
	// 	return nil, nil
	// }

	return Location{
		URI:   p.TextDocument.URI,
		Range: Range{}, //TODO(@s0cks): toRange(task.NameRange),
	}, nil
}
