package lsp

import (
	"fmt"
	"taskfile-lsp/internal/taskfile"
)

func CreateUndefinedTaskCodeAction(doc *Document, file *taskfile.File, params *CodeActionParams) ([]CodeAction, error) {
	actions := []CodeAction{}
	filter := NewDiagnosticCodePredicate(taskfile.CodeUndefinedTask)
	append_action := func(name string, diagnostic *Diagnostic) bool {
		actions = append(actions, CodeAction{
			Title:       fmt.Sprintf("Create task %q", name),
			Kind:        "quickfix",
			Diagnostics: []Diagnostic{*diagnostic},
			Edit: &WorkspaceEdit{
				Changes: map[string][]TextEdit{params.TextDocument.URI: {NewTaskEdit(doc.Parsed, name)}},
			},
		})

		return true
	}
	params.VisitDiagnosticsMatching(doc, filter, append_action)
	return actions, nil
}
