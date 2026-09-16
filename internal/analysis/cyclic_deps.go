package analysis

import (
	"strings"

	"taskfile-lsp/internal/document"
)

const CodeCyclicDependency = "cyclic-dependency"

type CyclicDepsPass struct{}

func (CyclicDepsPass) Name() string { return "cyclic-deps" }

func (CyclicDepsPass) Run(doc *document.Document) []Diagnostic {
	edges := map[string][]string{}
	tasks := map[string]document.Task{}

	doc.VisitTasks(func(_ uint64, t document.Task) bool {
		name := t.Name()
		tasks[name] = t
		n := t.GetNumberOfDeps()
		for i := range n {
			dep := t.GetDepAt(i)
			edges[name] = append(edges[name], dep.Name())
		}

		return true
	})

	const (
		white = 0
		gray  = 1
		black = 2
	)

	color := map[string]int{}
	seen := map[string]bool{} // dedupe: report each cycle once
	var diags []Diagnostic
	var stack []string

	var visit func(name string)
	visit = func(name string) {
		color[name] = gray
		stack = append(stack, name)

		for _, dep := range edges[name] {
			if _, exists := tasks[dep]; !exists {
				continue // undefined task -- MissingReferencesPass's job, not ours
			}

			switch color[dep] {
			case white:
				visit(dep)

			case gray:
				cycle := cycleFrom(stack, dep)
				key := strings.Join(cycle, ",")
				if !seen[key] {
					seen[key] = true
					t := tasks[name]
					diags = append(diags, Diagnostic{
						Range:   document.Range{Start: t.Start(), End: t.End()},
						Code:    CodeCyclicDependency,
						Message: "cyclic task dependency: " + strings.Join(cycle, " -> "),
					})
				}

			}
		}

		stack = stack[:len(stack)-1]
		color[name] = black
	}

	for name := range tasks {
		if color[name] == white {
			visit(name)
		}
	}

	return diags
}

func cycleFrom(stack []string, closingAt string) []string {
	for i, n := range stack {
		if n == closingAt {
			return append(append([]string{}, stack[i:]...), closingAt)
		}
	}

	return append(append([]string{}, stack...), closingAt)
}
