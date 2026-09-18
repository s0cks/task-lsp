package lsp

import (
	"fmt"
	"strings"

	"taskfile-lsp/internal/analysis"
	"taskfile-lsp/internal/document"
)

func RemoveIncompleteNodeCodeAction(doc *Document, uri string, resolve analysis.Resolver, params *CodeActionParams) []CodeAction {
	var actions []CodeAction

	doc.Access(func(parsed *document.Document, text string) {
		if parsed == nil {
			return
		}

		for _, d := range analysis.RunAll(uri, parsed, resolve, []analysis.Pass{analysis.IncompleteNodesPass{}}) {
			r := toLSPRange(d.Range)
			if !linesOverlap(r, params.Range) {
				continue
			}

			edit := wholeLinesRange(text, r.Start.Line, r.End.Line)
			actions = append(actions, CodeAction{
				Title:       fmt.Sprintf("Remove: %s", d.Message),
				Kind:        "quickfix",
				Diagnostics: []Diagnostic{{Range: r, Code: d.Code, Message: d.Message}},
				Edit: &WorkspaceEdit{
					Changes: map[string][]TextEdit{uri: {{Range: edit, NewText: ""}}},
				},
			})
		}
	})

	return actions
}

func toLSPPos(p document.Pos) Position {
	return Position{Line: p.Row, Character: p.Col}
}

func toLSPRange(r document.Range) Range {
	return Range{Start: toLSPPos(r.Start), End: toLSPPos(r.End)}
}

func linesOverlap(a, b Range) bool {
	return a.Start.Line <= b.End.Line && b.Start.Line <= a.End.Line
}

func wholeLinesRange(text string, startLine, endLine int) Range {
	lines := strings.Split(text, "\n")
	if startLine < 0 || endLine >= len(lines) || startLine > endLine {
		return Range{}
	}

	end := Position{Line: endLine + 1, Character: 0}
	if endLine+1 >= len(lines) {
		end = Position{Line: endLine, Character: len([]rune(lines[endLine]))}
	}

	return Range{Start: Position{Line: startLine, Character: 0}, End: end}
}
