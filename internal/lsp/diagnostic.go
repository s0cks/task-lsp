package lsp

import "taskfile-lsp/internal/analysis"

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

func (s *Server) computeDiagnosticsFor(uri string) []Diagnostic {
	f, ok := s.ws.File(uri)
	if !ok {
		return nil
	}

	if f.Parsed == nil {
		return []Diagnostic{{Severity: SeverityError, Message: "failed to parse document"}}
	}

	var diags []Diagnostic
	for _, d := range analysis.RunAll(uri, f.Parsed, s.ws, passes) {
		diags = append(diags, Diagnostic{
			Range:    toLSPRange(d.Range),
			Severity: SeverityWarning,
			Code:     d.Code,
			Message:  d.Message,
		})
	}

	for _, c := range s.ws.DetectIncludeCycles() {
		if c.URI != uri {
			continue
		}

		diags = append(diags, Diagnostic{
			Range:    toLSPRange(c.Range),
			Severity: SeverityWarning,
			Code:     "include-cycle",
			Message:  c.Message,
		})
	}

	return diags
}
