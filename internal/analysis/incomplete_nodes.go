package analysis

import "taskfile-lsp/internal/document"

const CodeIncompleteNode = "incomplete-node"

type IncompleteNodesPass struct{}

func (IncompleteNodesPass) Name() string { return "incomplete-nodes" }

// The parser marks incompleteness on the specific leaf that failed to parse
// (a bad "-" list item, an empty map-entry value) rather than propagating
// it up to the containing task or var. So this pass has to walk down into
// cmds/deps to find it -- checking only the task/var node itself would
// almost never report anything, even on genuinely malformed input.
func (IncompleteNodesPass) Run(uri string, doc *document.Document, resolve Resolver) []Diagnostic {
	var diags []Diagnostic

	report := func(r document.Range, what string) {
		diags = append(diags, Diagnostic{Range: r, Code: CodeIncompleteNode, Message: what})
	}

	doc.VisitTasks(func(_ uint64, t document.Task) bool {
		switch {
		case t.IsError():
			report(document.Range{Start: t.Start(), End: t.End()}, "task \""+t.Name()+"\" has a parse error")
			return true
		case t.IsIncomplete():
			report(document.Range{Start: t.Start(), End: t.End()}, "task \""+t.Name()+"\" is incomplete")
			return true
		}

		for i := uint64(0); i < t.GetNumberOfCmds(); i++ {
			cmd := t.GetCmdAt(i)
			if cmd.IsIncomplete() || cmd.IsError() {
				report(document.Range{Start: cmd.Start(), End: cmd.End()}, "task \""+t.Name()+"\" has an incomplete command")
			}
		}

		for i := uint64(0); i < t.GetNumberOfDeps(); i++ {
			dep := t.GetDepAt(i)
			if dep.IsIncomplete() || dep.IsError() {
				report(document.Range{Start: dep.Start(), End: dep.End()}, "task \""+t.Name()+"\" has an incomplete dependency")
			}
		}

		return true
	})

	doc.VisitVars(func(_ uint64, v document.Var) bool {
		switch {
		case v.IsError():
			report(document.Range{Start: v.Start(), End: v.End()}, "var \""+v.Name()+"\" has a parse error")
		case v.IsIncomplete():
			report(document.Range{Start: v.Start(), End: v.End()}, "var \""+v.Name()+"\" is incomplete")
		}
		return true
	})

	return diags
}
