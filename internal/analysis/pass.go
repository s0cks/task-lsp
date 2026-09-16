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
type Pass interface {
	Name() string
	Run(doc *document.Document) []Diagnostic
}

func RunAll(doc *document.Document, passes []Pass) []Diagnostic {
	var diags []Diagnostic
	for _, p := range passes {
		diags = append(diags, p.Run(doc)...)
	}
	return diags
}
