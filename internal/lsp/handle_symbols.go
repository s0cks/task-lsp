package lsp

import (
	"context"
	"encoding/json"
	"taskfile-lsp/internal/rpc"
)

func (s *Server) handleSymbols(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p SymbolsParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}

	symbols := []Symbol{}
	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok || doc.Parsed == nil {
		return symbols, nil
	}

	for _, v := range doc.Parsed.Vars {
		symbols = append(symbols, NewVarSymbol(v))
	}

	for _, task := range doc.Parsed.Tasks {
		symbols = append(symbols, NewTaskSymbol(task))
	}

	return symbols, nil
}
