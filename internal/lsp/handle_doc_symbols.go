package lsp

import (
	"context"
	"encoding/json"
	"taskfile-lsp/internal/document"
	"taskfile-lsp/internal/rpc"
)

func getSymbolsForTask(task *document.Task) []Symbol {
	symbols := []Symbol{}

	symbols = append(symbols, Symbol{
		Name:           task.Name(),
		Detail:         task.Desc(),
		Kind:           FunctionSymbol,
		Range:          toLSPRange(task.Range()),
		SelectionRange: toLSPRange(task.Range()),
	})

	task.VisitAliases(func(idx uint64, alias document.String) bool {
		symbols = append(symbols, Symbol{
			Name:           alias.Value(),
			Detail:         "",
			Kind:           FunctionSymbol,
			Range:          toLSPRange(task.Range()),
			SelectionRange: toLSPRange(task.Range()),
		})

		return true
	})

	return symbols
}

func getSymbolsForDoc(doc *document.Document) []Symbol {
	symbols := []Symbol{}

	doc.VisitTasks(func(idx uint64, task document.Task) bool {
		symbols = append(symbols, getSymbolsForTask(&task)...)
		return true
	})

	doc.VisitVars(func(idx uint64, v document.Var) bool {
		symbols = append(symbols, Symbol{
			Name:           v.Name(),
			Detail:         "",
			Kind:           VariableSymbol,
			Range:          toLSPRange(v.Range()),
			SelectionRange: toLSPRange(v.Range()),
		})

		return true
	})

	return symbols
}

func (s *Server) handleDocumentSymbols(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p SymbolsParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}

	symbols := []Symbol{}
	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok || doc.parsed == nil {
		return symbols, nil
	}

	symbols = append(symbols, getSymbolsForDoc(doc.parsed)...)
	return symbols, nil
}
