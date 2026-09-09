package lsp

import (
	"context"
	"encoding/json"
	"fmt"
	"taskfile-lsp/internal/rpc"
	"taskfile-lsp/internal/taskfile"
)

func (s *Server) handleHover(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p TextDocumentPositionParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}

	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok {
		return nil, nil
	}

	if doc.Parsed != nil {
		if name, kind, ok := doc.Parsed.NameOrRefAt(toTFPos(p.Position)); ok {
			switch kind {
			case taskfile.EntityTask:
				if t, ok := doc.Parsed.Tasks[name]; ok {
					value := fmt.Sprintf("**task: %s**", t.Name)
					if t.Desc != "" {
						value += "\n\n" + t.Desc
					}

					return Hover{Contents: MarkupContent{Kind: MarkupMarkdown, Value: value}}, nil
				}

			case taskfile.EntityVar:
				if v, ok := doc.Parsed.Vars[name]; ok {
					value := fmt.Sprintf("**var: %s**", v.Name)
					if v.Value != "" {
						value += "\n\n" + v.Value
					}

					return Hover{Contents: MarkupContent{Kind: MarkupMarkdown, Value: value}}, nil
				}

			}
		}
	}

	word := wordAt(doc.Text, p.Position)
	if word == "" {
		return nil, nil
	}

	return Hover{Contents: MarkupContent{Kind: MarkupMarkdown, Value: fmt.Sprintf("`%s`", word)}}, nil
}
