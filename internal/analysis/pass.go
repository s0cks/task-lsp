package analysis

import "taskfile-lsp/internal/document"

type Diagnostic struct {
	Range   document.Range
	Code    string
	Message string
}

// A Pass is one single-purpose check over a parsed document -- like a
// compiler optimization pass, it owns one concern and walks only the nodes
// it cares about. It is not a general visitor shared across passes.
//
// uri identifies which file doc came from, and resolve lets a pass follow
// a namespaced ref (e.g. a task dep like "docs:build") into another file's
// document without the pass needing to know anything about the workspace
// graph itself. Passes that don't need cross-file resolution (like
// IncompleteNodesPass) simply ignore both.
type Pass interface {
	Name() string
	Run(uri string, doc *document.Document, resolve Resolver) []Diagnostic
}

func RunAll(uri string, doc *document.Document, resolve Resolver, passes []Pass) []Diagnostic {
	var diags []Diagnostic
	for _, p := range passes {
		diags = append(diags, p.Run(uri, doc, resolve)...)
	}
	return diags
}
