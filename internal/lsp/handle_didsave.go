package lsp

import (
	"context"
	"encoding/json"

	"taskfile-lsp/internal/rpc"
)

func (s *Server) handleDidSave(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	var p DidSaveTextDocumentParams
	if err := json.Unmarshal(params, &p); err != nil {
		return
	}

	if doc, ok := s.docs.Get(p.TextDocument.URI); ok {
		s.publish(conn, doc.URI, doc.Version, s.diagCache[doc.URI])
	}
}
