package lsp

import (
	"context"
	"encoding/json"
	"taskfile-lsp/internal/rpc"
)

func (s *Server) handleInitialize(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p InitializeParams
	if len(params) > 0 {
		if err := json.Unmarshal(params, &p); err != nil {
			return nil, rpc.NewError(rpc.InvalidParams, err.Error())
		}
	}

	result := InitializeResult{
		Capabilities: ServerCapabilities{
			TextDocumentSync:       SyncFull,
			HoverProvider:          true,
			DocumentSymbolProvider: true,
			DefinitionProvider:     true,
			ReferencesProvider:     true,
			CodeActionProvider:     true,
			CompletionProvider: &CompletionOptions{
				TriggerCharacters: []string{"-"},
			},
		},
		ServerInfo: &ServerInfo{Name: "taskfile-lsp", Version: "0.1.0"},
	}

	return result, nil
}
