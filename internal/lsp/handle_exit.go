package lsp

import (
	"context"
	"encoding/json"
	"os"
	"taskfile-lsp/internal/rpc"
)

func (s *Server) handleExit(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	os.Exit(0)
}
