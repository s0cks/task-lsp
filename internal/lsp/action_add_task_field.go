package lsp

import (
	"strings"
	"taskfile-lsp/internal/taskfile"
)

func GenRefactorTaskActions(doc *Document, file *taskfile.File, params *CodeActionParams) ([]CodeAction, error) {
	actions := []CodeAction{}
	if task, ok := doc.Parsed.TaskAt(toTFPos(params.Range.Start)); ok && task.Desc == "" {
		at := Position{Line: task.DefLine + 1, Character: 0}
		actions = append(actions, CodeAction{
			Title: "Add description",
			Kind:  "refactor",
			Edit: &WorkspaceEdit{
				Changes: map[string][]TextEdit{params.TextDocument.URI: {{
					Range:   Range{Start: at, End: at},
					NewText: strings.Repeat(" ", doc.Parsed.BodyIndent) + "desc: \"\"\n",
				}}},
			},
		})
	}

	return actions, nil
}
