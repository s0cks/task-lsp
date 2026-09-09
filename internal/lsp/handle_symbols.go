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
		symbols = append(symbols, Symbol{
			Name:   v.Name,
			Detail: "property",
			Kind:   7,
			SelectionRange: Range{
				Start: Position{
					Line:      v.NameRange.Start.Line,
					Character: v.NameRange.Start.Character,
				},
				End: Position{
					Line:      v.NameRange.Start.Line,
					Character: v.NameRange.Start.Character,
				},
			},
			Range: Range{
				Start: Position{
					Line:      v.NameRange.Start.Line,
					Character: v.NameRange.Start.Character,
				},
				End: Position{
					Line:      v.NameRange.Start.Line,
					Character: v.NameRange.Start.Character,
				},
			},
		})
	}

	for _, task := range doc.Parsed.Tasks {
		symbols = append(symbols, Symbol{
			Name:   task.Name,
			Detail: "function",
			Kind:   12,
			SelectionRange: Range{
				Start: Position{
					Line:      task.NameRange.Start.Line,
					Character: task.NameRange.Start.Character,
				},
				End: Position{
					Line:      task.NameRange.Start.Line,
					Character: task.NameRange.Start.Character,
				},
			},
			Range: Range{
				Start: Position{
					Line:      task.NameRange.Start.Line,
					Character: task.NameRange.Start.Character,
				},
				End: Position{
					Line:      task.NameRange.Start.Line,
					Character: task.NameRange.Start.Character,
				},
			},
		})
	}

	return symbols, nil
}
