package lsp

import "taskfile-lsp/internal/taskfile"

type Symbol struct {
	Name           string `json:"name"`
	Detail         string `json:"detail"`
	Kind           int    `json:"kind"`
	Range          Range  `json:"range"`
	SelectionRange Range  `json:"selectionRange"`
}

func NewTaskSymbol(t *taskfile.Task) Symbol {
	return Symbol{
		Name:   t.Name,
		Detail: "function",
		Kind:   12,
		SelectionRange: Range{
			Start: Position{
				Line:      t.NameRange.Start.Line,
				Character: t.NameRange.Start.Character,
			},
			End: Position{
				Line:      t.NameRange.Start.Line,
				Character: t.NameRange.Start.Character,
			},
		},
		Range: Range{
			Start: Position{
				Line:      t.NameRange.Start.Line,
				Character: t.NameRange.Start.Character,
			},
			End: Position{
				Line:      t.NameRange.Start.Line,
				Character: t.NameRange.Start.Character,
			},
		},
	}
}

func NewVarSymbol(v *taskfile.TaskVar) Symbol {
	return Symbol{
		Name:   v.Name,
		Detail: "property",
		Kind:   7,
		SelectionRange: Range{
			Start: Position{
				Line:      v.NameRange.Start.Line,
				Character: v.NameRange.Start.Character,
			},
			End: Position{
				Line:      v.NameRange.Start.Line,
				Character: v.NameRange.Start.Character,
			},
		},
		Range: Range{
			Start: Position{
				Line:      v.NameRange.Start.Line,
				Character: v.NameRange.Start.Character,
			},
			End: Position{
				Line:      v.NameRange.Start.Line,
				Character: v.NameRange.Start.Character,
			},
		},
	}
}
