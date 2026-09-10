package lsp

import (
	"context"
	"encoding/json"
	"taskfile-lsp/internal/rpc"
)

func (s *Server) handleDidClose(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	var p DidCloseTextDocumentParams
	if err := json.Unmarshal(params, &p); err != nil {
		return
	}

	s.docs.Close(p.TextDocument.URI)
}
