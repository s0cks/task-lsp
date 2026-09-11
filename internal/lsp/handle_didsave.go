package lsp

import (
	"context"
	"encoding/json"
	"taskfile-lsp/internal/rpc"
	"taskfile-lsp/internal/taskfile"
)

func (s *Server) handleDidSave(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	var p DidSaveTextDocumentParams
	if err := json.Unmarshal(params, &p); err != nil {
		return
	}

	if doc, ok := s.docs.Get(p.TextDocument.URI); ok {
		parser := taskfile.Parser{Log: s.log}
		_, diags := parser.Parse(doc.Text)
		s.publish(conn, doc.URI, doc.Version, diags)
	}
}
