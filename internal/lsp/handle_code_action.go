package lsp

import (
	"context"
	"encoding/json"
	"fmt"
	"strings"
	"taskfile-lsp/internal/rpc"
	"taskfile-lsp/internal/taskfile"
)

func (s *Server) handleCodeAction(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p CodeActionParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}

	actions := []CodeAction{}
	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok || doc.Parsed == nil {
		return actions, nil
	}

	for _, d := range p.Context.Diagnostics {
		if d.Code != taskfile.CodeUndefinedTask {
			continue
		}

		name := strings.TrimSpace(textInRange(doc.Text, d.Range))
		if name == "" {
			continue
		}

		actions = append(actions, CodeAction{
			Title:       fmt.Sprintf("Create task %q", name),
			Kind:        "quickfix",
			Diagnostics: []Diagnostic{d},
			Edit: &WorkspaceEdit{
				Changes: map[string][]TextEdit{p.TextDocument.URI: {newTaskEdit(doc.Parsed, name)}},
			},
		})
	}

	if task, ok := doc.Parsed.TaskAt(toTFPos(p.Range.Start)); ok && task.Desc == "" {
		at := Position{Line: task.DefLine + 1, Character: 0}
		actions = append(actions, CodeAction{
			Title: "Add description",
			Kind:  "refactor",
			Edit: &WorkspaceEdit{
				Changes: map[string][]TextEdit{p.TextDocument.URI: {{
					Range:   Range{Start: at, End: at},
					NewText: strings.Repeat(" ", doc.Parsed.BodyIndent) + "desc: \"\"\n",
				}}},
			},
		})
	}

	return actions, nil
}
