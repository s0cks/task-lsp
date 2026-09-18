package analysis

import "taskfile-lsp/internal/document"

type Resolver interface {
	FindTask(uri, ref string) (document.Task, string, bool)
	FindVar(uri, ref string) (document.Var, string, bool)
}
