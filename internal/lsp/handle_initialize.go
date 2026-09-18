package lsp

import (
	"context"
	"encoding/json"
	"os"
	"path/filepath"

	"taskfile-lsp/internal/rpc"
	"taskfile-lsp/internal/workspace"

	"charm.land/log/v2"
)

func (s *Server) handleInitialize(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p InitializeParams
	if len(params) > 0 {
		if err := json.Unmarshal(params, &p); err != nil {
			return nil, rpc.NewError(rpc.InvalidParams, err.Error())
		}
	}

	rootURI := p.RootURI
	if rootURI == nil && len(p.WorkspaceFolders) > 0 {
		rootURI = &p.WorkspaceFolders[0].URI
	}

	if rootURI != nil {
		if rootPath, err := workspace.URIToPath(*rootURI); err == nil {
			s.loadWorkspaceRoot(rootPath)
		} else {
			log.Errorf("initialize: could not resolve root URI %q: %v", *rootURI, err)
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

func (s *Server) loadWorkspaceRoot(rootPath string) {
	s.ws.SetRoot(rootPath)

	names := []string{"Taskfile.yml", "Taskfile.yaml", "taskfile.yml", "taskfile.yaml"}
	for _, name := range names {
		path := filepath.Join(rootPath, name)
		data, err := os.ReadFile(path)
		if err != nil {
			continue
		}

		uri := workspace.PathToURI(path)
		s.ws.Load(uri, 0, string(data), false)
		return
	}
}
