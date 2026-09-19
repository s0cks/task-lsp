package analysis

import (
	"fmt"
	"taskfile-lsp/internal/document"
)

const CodeIncompleteNode = "incomplete-node"

type IncompleteNodesPass struct{}

func (IncompleteNodesPass) Name() string { return "incomplete-nodes" }

type HasRange interface {
	Range() document.Range
}

func (IncompleteNodesPass) Run(uri string, doc *document.Document, resolve Resolver) []Diagnostic {
	var diags []Diagnostic

	report := func(t HasRange, format string, args ...any) {
		diags = append(diags, Diagnostic{
			Range:   t.Range(),
			Code:    CodeIncompleteNode,
			Message: fmt.Sprintf(format, args...),
		})
	}

	doc.VisitTasks(func(_ uint64, t document.Task) bool {
		switch {
		case t.IsError():
			report(&t, "task `%s` has a parse error", t.Name())
			return true

		case t.IsIncomplete():
			report(&t, "task `%s` is incomplete", t.Name())
			return true
		}

		for i := uint64(0); i < t.GetNumberOfCmds(); i++ {
			cmd := t.GetCmdAt(i)
			if cmd.IsIncomplete() || cmd.IsError() {
				report(&cmd, "task `%s` has incomplete command", t.Name())
			}
		}

		for i := uint64(0); i < t.GetNumberOfDeps(); i++ {
			dep := t.GetDepAt(i)
			if dep.IsIncomplete() || dep.IsError() {
				report(&dep, "task `%s` has incomplete dependency", t.Name())
			}
		}

		return true
	})

	doc.VisitVars(func(_ uint64, v document.Var) bool {
		switch {
		case v.IsError():
			report(&v, "var `%s` has a parse error", v.Name())

		case v.IsIncomplete():
			report(&v, "var `%s` is incomplete", v.Name())
		}

		return true
	})

	return diags
}
