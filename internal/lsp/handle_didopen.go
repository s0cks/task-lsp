package lsp

import (
	"context"
	"encoding/json"
	"taskfile-lsp/internal/rpc"
)

func (s *Server) handleDidOpen(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	var p DidOpenTextDocumentParams
	if err := json.Unmarshal(params, &p); err != nil {
		s.log.Printf("didOpen: %v", err)
		return
	}

	diags := s.docs.Open(&Document{
		URI:        p.TextDocument.URI,
		LanguageID: p.TextDocument.LanguageID,
		Version:    p.TextDocument.Version,
		Text:       p.TextDocument.Text,
	})
	s.publish(conn, p.TextDocument.URI, p.TextDocument.Version, diags)
}
