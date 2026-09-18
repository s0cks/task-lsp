package lsp

import (
	"context"
	"encoding/json"

	"taskfile-lsp/internal/rpc"

	"charm.land/log/v2"
)

func (s *Server) handleDidOpen(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	var p DidOpenTextDocumentParams
	if err := json.Unmarshal(params, &p); err != nil {
		log.Errorf("didOpen: %v", err)
		return
	}

	affected := s.docs.Open(&Document{
		URI:        p.TextDocument.URI,
		LanguageID: p.TextDocument.LanguageID,
		Version:    p.TextDocument.Version,
		Text:       p.TextDocument.Text,
	})
	s.recomputeAndPublish(conn, affected)
}
