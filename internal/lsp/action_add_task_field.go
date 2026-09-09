package lsp

import (
	"strings"
	"taskfile-lsp/internal/taskfile"
)

type WorkspaceEditGenerator func(uri string, doc *Document, task *taskfile.Task, at Position) *WorkspaceEdit

func addTaskFieldEdit(field string) WorkspaceEditGenerator {
	return func(uri string, doc *Document, task *taskfile.Task, at Position) *WorkspaceEdit {
		return &WorkspaceEdit{
			Changes: map[string][]TextEdit{uri: {{
				Range:   Range{Start: at, End: at},
				NewText: strings.Repeat(" ", doc.Parsed.BodyIndent) + field + ": \"\"\n",
			}}},
		}
	}
}

func NewRefactorTaskCodeAction(title string, uri string, doc *Document, task *taskfile.Task, at Position, gen WorkspaceEditGenerator) CodeAction {
	return CodeAction{
		Title: title,
		Kind:  "refactor",
		Edit:  gen(uri, doc, task, at),
	}
}

func getTaskCodeActions(doc *Document, params *CodeActionParams, task *taskfile.Task) []CodeAction {
	actions := []CodeAction{}
	at := Position{Line: task.DefLine + 1, Character: 0}
	if task.Desc == "" {
		actions = append(actions, NewRefactorTaskCodeAction("Add description", params.TextDocument.URI, doc, task, at, addTaskFieldEdit("desc")))
	}

	if task.Summary == "" {
		actions = append(actions, NewRefactorTaskCodeAction("Add summary", params.TextDocument.URI, doc, task, at, addTaskFieldEdit("summary")))
	}

	if task.Label == "" {
		actions = append(actions, NewRefactorTaskCodeAction("Add label", params.TextDocument.URI, doc, task, at, addTaskFieldEdit("label")))
	}

	return actions
}

func GenRefactorTaskActions(doc *Document, file *taskfile.File, params *CodeActionParams) ([]CodeAction, error) {
	actions := []CodeAction{}
	if task, ok := doc.Parsed.TaskAt(toTFPos(params.Range.Start)); ok {
		actions = append(actions, getTaskCodeActions(doc, params, task)...)
	}

	return actions, nil
}
