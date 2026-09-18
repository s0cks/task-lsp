package lsp

import (
	"context"
	"encoding/json"
	"taskfile-lsp/internal/rpc"

	"charm.land/log/v2"
)

func (s *Server) handleInitialized(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	log.Info("client initialized")
}
