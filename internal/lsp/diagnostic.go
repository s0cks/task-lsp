package lsp

import "taskfile-lsp/internal/analysis"
import "taskfile-lsp/internal/document"

type DiagnosticSeverity int

const (
	SeverityError DiagnosticSeverity = iota + 1
	SeverityWarning
	SeverityInformation
	SeverityHint
)

type Diagnostic struct {
	Range    Range              `json:"range"`
	Severity DiagnosticSeverity `json:"severity,omitempty"`
	Code     string             `json:"code,omitempty"`
	Source   string             `json:"source,omitempty"`
	Message  string             `json:"message"`
}

type DiagnosticVisitor func(name string, diagnostic *Diagnostic) bool

type DiagnosticPredicate func(name string, diagnostic *Diagnostic) bool

func NewDiagnosticCodePredicate(code string) DiagnosticPredicate {
	return func(name string, diagnostic *Diagnostic) bool {
		return diagnostic.Code == code
	}
}

type PublishDiagnosticsParams struct {
	URI         string       `json:"uri"`
	Version     *int         `json:"version,omitempty"`
	Diagnostics []Diagnostic `json:"diagnostics"`
}

var passes = []analysis.Pass{
	analysis.IncompleteNodesPass{},
	analysis.MissingReferencesPass{},
	analysis.CyclicDepsPass{},
}

// parseAndConvert parses text into a *document.Document and runs every
// registered pass over it, converting the result into LSP diagnostics.
// On parse failure it returns a single diagnostic built from the parser's
// error message and a nil *document.Document -- callers must treat a nil
// parsed doc as "nothing to query" rather than a programmer error.
func parseAndConvert(text string) (*document.Document, []Diagnostic) {
	parsed, err := document.ParseDocumentString(text)
	if err != nil {
		return nil, []Diagnostic{{
			Range:    Range{},
			Severity: SeverityError,
			Message:  err.Error(),
		}}
	}

	var diags []Diagnostic
	for _, d := range analysis.RunAll(parsed, passes) {
		diags = append(diags, Diagnostic{
			Range:    toLSPRange(d.Range),
			Severity: SeverityWarning,
			Code:     d.Code,
			Message:  d.Message,
		})
	}

	return parsed, diags
}
