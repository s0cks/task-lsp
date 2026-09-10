package lsp

import (
	"context"
	"encoding/json"
	"taskfile-lsp/internal/rpc"
)

func (s *Server) handleReferences(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p ReferenceParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}

	locs := []Location{}
	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok || doc.Parsed == nil {
		return locs, rpc.NewErrorf(rpc.InternalError, "document is unparsed")
	}

	name, _, ok := doc.Parsed.NameOrRefAt(toTFPos(p.Position))
	if !ok {
		return locs, rpc.NewErrorf(rpc.InternalError, "failed to find reference at pos: %v", p.Position)
	}

	if p.Context.IncludeDeclaration {
		if t, ok := doc.Parsed.Tasks[name]; ok {
			locs = append(locs, Location{URI: p.TextDocument.URI, Range: toRange(t.NameRange)})
		}
	}

	for _, r := range doc.Parsed.Refs {
		if r.Name == name {
			locs = append(locs, Location{URI: p.TextDocument.URI, Range: toRange(r.Range)})
		}
	}

	return locs, nil
}
