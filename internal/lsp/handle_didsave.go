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

	// The stored Diagnostics are already current as of the last didChange --
	// LSP guarantees didChange has synced the saved content, so there's
	// nothing new to parse here. Re-parsing anyway would mean a second full
	// CGo parse of text we've already parsed, for no new information.
	if doc, ok := s.docs.Get(p.TextDocument.URI); ok {
		s.publish(conn, doc.URI, doc.Version, doc.Diagnostics)
	}
}
