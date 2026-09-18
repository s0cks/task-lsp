package analysis

import "taskfile-lsp/internal/document"

// Resolver looks up a task/var ref (bare "name" or namespaced "ns:name")
// from the perspective of a given file, hopping across an include graph
// for namespaced refs. Kept as an interface here so analysis doesn't
// depend on internal/workspace -- the workspace package implements it.
type Resolver interface {
	FindTask(uri, ref string) (document.Task, string, bool)
	FindVar(uri, ref string) (document.Var, string, bool)
}
