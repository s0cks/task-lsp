package analysis

import (
	"fmt"
	"taskfile-lsp/internal/document"
)

const CodeMissingReference = "missing-reference"

type MissingReferencesPass struct{}

func (MissingReferencesPass) Name() string { return "missing-references" }

func (MissingReferencesPass) Run(uri string, doc *document.Document, resolve Resolver) []Diagnostic {
	var diags []Diagnostic

	doc.VisitVars(func(_ uint64, v document.Var) bool {
		if !v.IsRef() {
			return true
		}

		ref := v.Ref()
		if _, _, ok := resolve.FindVar(uri, ref.Name()); !ok {
			diags = append(diags, Diagnostic{
				Range:   ref.Range(),
				Code:    CodeMissingReference,
				Message: fmt.Sprintf("var `%s` references undefined var `%s`", v.Name(), ref.Name()),
			})
		}

		return true
	})

	doc.VisitTasks(func(_ uint64, t document.Task) bool {
		n := t.GetNumberOfDeps()
		for i := range n {
			dep := t.GetDepAt(i)
			if _, _, ok := resolve.FindTask(uri, dep.Name()); !ok {
				diags = append(diags, Diagnostic{
					Range:   dep.Range(),
					Code:    CodeMissingReference,
					Message: fmt.Sprintf("task `%s` depends on undefined task `%s`", t.Name(), dep.Name()),
				})
			}
		}

		return true
	})

	return diags
}
