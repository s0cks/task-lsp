package analysis

import "taskfile-lsp/internal/document"

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
				Message: "var \"" + v.Name() + "\" references undefined var \"" + ref.Name() + "\"",
			})
		}
		return true
	})

	doc.VisitTasks(func(_ uint64, t document.Task) bool {
		n := t.GetNumberOfDeps()
		for i := uint64(0); i < n; i++ {
			dep := t.GetDepAt(i)
			if _, _, ok := resolve.FindTask(uri, dep.Name()); !ok {
				diags = append(diags, Diagnostic{
					Range:   dep.Range(),
					Code:    CodeMissingReference,
					Message: "task \"" + t.Name() + "\" depends on undefined task \"" + dep.Name() + "\"",
				})
			}
		}
		return true
	})

	return diags
}
