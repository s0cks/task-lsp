package lsp

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
