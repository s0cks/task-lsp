package workspace

import (
	"os"
	"strings"

	"taskfile-lsp/internal/document"
)

type IncludeEdge struct {
	Namespace string
	TargetURI string
	Optional  bool
	Range     document.Range
}

type File struct {
	URI  string
	Path string

	Open    bool
	Version int
	Text    string

	Parsed *document.Document

	IncludeEdges []IncludeEdge
	IncludedBy   map[string]bool
}

type Workspace struct {
	root  string
	files map[string]*File
}

func New() *Workspace {
	return &Workspace{files: make(map[string]*File)}
}

func (w *Workspace) SetRoot(path string) {
	w.root = path
}

func (w *Workspace) Root() string {
	return w.root
}

func (w *Workspace) File(uri string) (*File, bool) {
	f, ok := w.files[uri]
	return f, ok
}

func (w *Workspace) URIs() []string {
	out := make([]string, 0, len(w.files))
	for u := range w.files {
		out = append(out, u)
	}

	return out
}

func (w *Workspace) Load(uri string, version int, text string, open bool) []string {
	f := w.files[uri]
	if f == nil {
		f = &File{URI: uri, IncludedBy: map[string]bool{}}
		w.files[uri] = f
	}

	if f.Path == "" {
		if p, err := URIToPath(uri); err == nil {
			f.Path = p
		}
	}

	old := f.Parsed
	parsed, _ := document.ParseDocumentString(text)

	f.Text = text
	f.Version = version
	f.Parsed = parsed
	if open {
		f.Open = true
	}

	for _, e := range f.IncludeEdges {
		if target, ok := w.files[e.TargetURI]; ok {
			delete(target.IncludedBy, uri)
		}
	}

	f.IncludeEdges = nil

	if parsed != nil && f.Path != "" {
		parsed.VisitIncludes(func(_ uint64, inc document.Include) bool {
			edge := IncludeEdge{
				Namespace: inc.Namespace(),
				Optional:  inc.IsOptional(),
				Range:     document.Range{Start: inc.Start(), End: inc.End()},
			}

			if targetPath, err := resolveIncludePath(f.Path, inc); err == nil {
				targetURI := PathToURI(targetPath)
				edge.TargetURI = targetURI

				if _, tracked := w.files[targetURI]; !tracked {
					if data, err := os.ReadFile(targetPath); err == nil {
						w.Load(targetURI, 0, string(data), false)
					}
				}

				if target, ok := w.files[targetURI]; ok {
					target.IncludedBy[uri] = true
				}
			}

			f.IncludeEdges = append(f.IncludeEdges, edge)
			return true
		})
	}

	if old != nil {
		old.Free()
	}

	return w.affected(uri)
}

func (w *Workspace) Close(uri string) {
	if f, ok := w.files[uri]; ok {
		f.Open = false
	}
}

func (w *Workspace) affected(uri string) []string {
	seen := map[string]bool{uri: true}
	queue := []string{uri}

	for len(queue) > 0 {
		cur := queue[0]
		queue = queue[1:]

		f, ok := w.files[cur]
		if !ok {
			continue
		}

		for parent := range f.IncludedBy {
			if !seen[parent] {
				seen[parent] = true
				queue = append(queue, parent)
			}
		}
	}

	out := make([]string, 0, len(seen))
	for u := range seen {
		out = append(out, u)
	}

	return out
}

func splitRef(ref string) (namespace, name string, hasNamespace bool) {
	for i := 0; i < len(ref); i++ {
		if ref[i] == ':' {
			return ref[:i], ref[i+1:], true
		}
	}

	return "", ref, false
}

func (w *Workspace) FindTask(uri, ref string) (document.Task, string, bool) {
	f, ok := w.files[uri]
	if !ok || f.Parsed == nil {
		return document.Task{}, "", false
	}

	ns, name, hasNS := splitRef(ref)
	if !hasNS {
		t, ok := f.Parsed.FindTask(name)
		return t, uri, ok
	}

	for _, e := range f.IncludeEdges {
		if e.Namespace != ns || e.TargetURI == "" {
			continue
		}

		target, ok := w.files[e.TargetURI]
		if !ok || target.Parsed == nil {
			continue
		}

		t, ok := target.Parsed.FindTask(name)
		return t, e.TargetURI, ok
	}

	return document.Task{}, "", false
}

func (w *Workspace) FindVar(uri, ref string) (document.Var, string, bool) {
	f, ok := w.files[uri]
	if !ok || f.Parsed == nil {
		return document.Var{}, "", false
	}

	ns, name, hasNS := splitRef(ref)
	if !hasNS {
		v, ok := f.Parsed.FindVar(name)
		return v, uri, ok
	}

	for _, e := range f.IncludeEdges {
		if e.Namespace != ns || e.TargetURI == "" {
			continue
		}

		target, ok := w.files[e.TargetURI]
		if !ok || target.Parsed == nil {
			continue
		}

		v, ok := target.Parsed.FindVar(name)
		return v, e.TargetURI, ok
	}

	return document.Var{}, "", false
}

type IncludeCycle struct {
	URI     string
	Range   document.Range
	Message string
}

func (w *Workspace) DetectIncludeCycles() []IncludeCycle {
	const (
		white = 0
		gray  = 1
		black = 2
	)

	color := map[string]int{}
	seen := map[string]bool{}
	var cycles []IncludeCycle
	var stack []string

	var visit func(uri string)
	visit = func(uri string) {
		color[uri] = gray
		stack = append(stack, uri)

		f, ok := w.files[uri]
		if ok {
			for _, e := range f.IncludeEdges {
				if e.TargetURI == "" {
					continue
				}

				switch color[e.TargetURI] {
				case white:
					visit(e.TargetURI)

				case gray:
					cycle := cycleFromURI(stack, e.TargetURI)
					var key strings.Builder
					for _, u := range cycle {
						key.WriteString(u)
						key.WriteString(",")
					}

					if !seen[key.String()] {
						seen[key.String()] = true
						cycles = append(cycles, IncludeCycle{
							URI:     uri,
							Range:   e.Range,
							Message: "cyclic include: " + describeCycleURIs(cycle),
						})
					}
				}
			}
		}

		stack = stack[:len(stack)-1]
		color[uri] = black
	}

	for uri := range w.files {
		if color[uri] == white {
			visit(uri)
		}
	}

	return cycles
}

func cycleFromURI(stack []string, closingAt string) []string {
	for i, u := range stack {
		if u == closingAt {
			return append(append([]string{}, stack[i:]...), closingAt)
		}
	}

	return append(append([]string{}, stack...), closingAt)
}

func describeCycleURIs(cycle []string) string {
	var out strings.Builder
	for i, u := range cycle {
		if i > 0 {
			out.WriteString(" -> ")
		}

		if p, err := URIToPath(u); err == nil {
			out.WriteString(p)
		} else {
			out.WriteString(u)
		}
	}

	return out.String()
}
