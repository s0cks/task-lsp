package lsp

import (
	"context"
	"encoding/json"
	"taskfile-lsp/internal/rpc"
)

func (s *Server) handleShutdown(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	return nil, nil
}
