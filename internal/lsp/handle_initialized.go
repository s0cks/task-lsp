package lsp

import (
	"context"
	"encoding/json"
	"taskfile-lsp/internal/rpc"
)

func (s *Server) handleInitialized(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	s.log.Println("client initialized")
}
