package analysis

import (
	"fmt"
	"taskfile-lsp/internal/document"
)

const CodeMissingReference = "missing-reference"

type MissingReferencesPass struct{}

func (MissingReferencesPass) Name() string { return "missing-references" }

func (MissingReferencesPass) Run(doc *document.Document) []Diagnostic {
	var diags []Diagnostic

	doc.VisitVars(func(_ uint64, v document.Var) bool {
		if !v.IsRef() {
			return true // only ref-kind vars point at another var -- skip the rest
		}

		ref := v.Ref()
		if _, ok := doc.FindVar(ref.Name()); !ok {
			diags = append(diags, Diagnostic{
				Range:   ref.Range(),
				Code:    CodeMissingReference,
				Message: "var \"" + v.Name() + "\" references undefined var \"" + ref.Name() + "\"",
			})
		}

		return true
	})

	doc.VisitTasks(func(_ uint64, t document.Task) bool {
		t.VisitDeps(func(_ uint64, r document.Ref) bool {
			if _, ok := doc.FindTask(r.Name()); !ok {
				diags = append(diags, Diagnostic{
					Range:   r.Range(),
					Code:    CodeMissingReference,
					Message: fmt.Sprintf("task `%s` depends on undefined task `%s`", t.Name(), r.Name()),
				})
			}

			return true
		})

		return true
	})

	return diags
}
