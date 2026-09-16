package analysis

import "taskfile-lsp/internal/document"

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
		n := t.GetNumberOfDeps()
		for i := uint64(0); i < n; i++ {
			dep := t.GetDepAt(i)
			if _, ok := doc.FindTask(dep.Name()); !ok {
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
