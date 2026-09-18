package analysis

import "taskfile-lsp/internal/document"

type Diagnostic struct {
	Range   document.Range
	Code    string
	Message string
}

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
