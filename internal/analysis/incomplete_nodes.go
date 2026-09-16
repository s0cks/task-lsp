package analysis

import "taskfile-lsp/internal/document"

const CodeIncompleteNode = "incomplete-node"

type IncompleteNodesPass struct{}

func (IncompleteNodesPass) Name() string { return "incomplete-nodes" }

func (IncompleteNodesPass) Run(doc *document.Document) []Diagnostic {
	var diags []Diagnostic

	doc.VisitTasks(func(_ uint64, t document.Task) bool {
		switch {
		case t.IsError():
			diags = append(diags, Diagnostic{
				Range:   document.Range{Start: t.Start(), End: t.End()},
				Code:    CodeIncompleteNode,
				Message: "task \"" + t.Name() + "\" has a parse error",
			})
		case t.IsIncomplete():
			diags = append(diags, Diagnostic{
				Range:   document.Range{Start: t.Start(), End: t.End()},
				Code:    CodeIncompleteNode,
				Message: "task \"" + t.Name() + "\" is incomplete",
			})
		}
		return true // every other task is fine -- skip it, nothing to report
	})

	doc.VisitVars(func(_ uint64, v document.Var) bool {
		switch {
		case v.IsError():
			diags = append(diags, Diagnostic{
				Range:   document.Range{Start: v.Start(), End: v.End()},
				Code:    CodeIncompleteNode,
				Message: "var \"" + v.Name() + "\" has a parse error",
			})
		case v.IsIncomplete():
			diags = append(diags, Diagnostic{
				Range:   document.Range{Start: v.Start(), End: v.End()},
				Code:    CodeIncompleteNode,
				Message: "var \"" + v.Name() + "\" is incomplete",
			})
		}
		return true
	})

	return diags
}
