package analysis

import (
	"strings"

	"taskfile-lsp/internal/document"
)

const CodeCyclicDependency = "cyclic-dependency"

type CyclicDepsPass struct{}

func (CyclicDepsPass) Name() string { return "cyclic-deps" }

type depNode struct {
	uri  string
	name string
}

func (CyclicDepsPass) Run(uri string, doc *document.Document, resolve Resolver) []Diagnostic {
	tasks := map[depNode]document.Task{}
	doc.VisitTasks(func(_ uint64, t document.Task) bool {
		tasks[depNode{uri, t.Name()}] = t
		return true
	})

	const (
		white = 0
		gray  = 1
		black = 2
	)
	color := map[depNode]int{}
	seen := map[string]bool{}
	var diags []Diagnostic
	var stack []depNode

	var visit func(n depNode, t document.Task)
	visit = func(n depNode, t document.Task) {
		color[n] = gray
		stack = append(stack, n)

		for i := uint64(0); i < t.GetNumberOfDeps(); i++ {
			dep := t.GetDepAt(i)
			ref := dep.Name()
			dt, targetURI, ok := resolve.FindTask(n.uri, ref)
			if !ok {
				continue
			}

			dn := depNode{uri: targetURI, name: dt.Name()}

			switch color[dn] {
			case white:
				tasks[dn] = dt
				visit(dn, dt)

			case gray:
				cycle := cycleFrom(stack, dn)
				key := cycleKey(cycle)
				if !seen[key] {
					seen[key] = true
					diags = append(diags, Diagnostic{
						Range:   document.Range{Start: t.Start(), End: t.End()},
						Code:    CodeCyclicDependency,
						Message: "cyclic task dependency: " + describeCycle(cycle),
					})
				}

			}
		}

		stack = stack[:len(stack)-1]
		color[n] = black
	}

	for n, t := range tasks {
		if color[n] == white {
			visit(n, t)
		}
	}

	return diags
}

func cycleFrom(stack []depNode, closingAt depNode) []depNode {
	for i, n := range stack {
		if n == closingAt {
			return append(append([]depNode{}, stack[i:]...), closingAt)
		}
	}

	return append(append([]depNode{}, stack...), closingAt)
}

func cycleKey(cycle []depNode) string {
	var b strings.Builder
	for _, n := range cycle {
		b.WriteString(n.uri)
		b.WriteByte(':')
		b.WriteString(n.name)
		b.WriteByte(',')
	}

	return b.String()
}

func describeCycle(cycle []depNode) string {
	var b strings.Builder
	for i, n := range cycle {
		if i > 0 {
			b.WriteString(" -> ")
		}

		b.WriteString(n.name)
	}

	return b.String()
}
