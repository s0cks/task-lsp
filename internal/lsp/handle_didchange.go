package lsp

import (
	"context"
	"encoding/json"
	"taskfile-lsp/internal/rpc"
)

func (s *Server) handleDidChange(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	var p DidChangeTextDocumentParams
	if err := json.Unmarshal(params, &p); err != nil {
		s.log.Printf("didChange: %v", err)
		return
	}

	if len(p.ContentChanges) == 0 {
		return
	}

	text := p.ContentChanges[len(p.ContentChanges)-1].Text
	diags := s.docs.Update(p.TextDocument.URI, p.TextDocument.Version, text)
	s.publish(conn, p.TextDocument.URI, p.TextDocument.Version, diags)
}
