package taskfile

import (
	"fmt"
	"regexp"
	"strings"
)

var (
	keyLineRe  = regexp.MustCompile(`^(\s*)([A-Za-z0-9_.$-]+):(?:(\s+)(.*))?\s*$`)
	listItemRe = regexp.MustCompile(`^(\s*)-(\s+)(.*)$`)
	taskCallRe = regexp.MustCompile(`^task:\s*(\S+)`)
)

type frame struct {
	indent   int
	kind     string
	taskName string
	varName  string
}

func Parse(content string) (*File, []Diagnostic) {
	lines := strings.Split(content, "\n")
	for i, l := range lines {
		lines[i] = strings.TrimRight(l, "\r")
	}

	f := &File{
		Tasks:       make(map[string]*Task),
		Vars:        make(map[string]*TaskVar),
		Includes:    make(map[string]string),
		lineContext: make([]LineContext, len(lines)),
	}

	var diags []Diagnostic
	stack := []frame{{indent: -1, kind: "root"}}

	for i, raw := range lines {
		trimmed := strings.TrimSpace(raw)
		if trimmed == "" || strings.HasPrefix(trimmed, "#") {
			top := stack[len(stack)-1]
			f.lineContext[i] = LineContext{Kind: top.kind, TaskName: top.taskName}
			continue
		}

		indent := len(raw) - len(strings.TrimLeft(raw, " "))
		for len(stack) > 1 && indent <= stack[len(stack)-1].indent {
			stack = stack[:len(stack)-1]
		}

		top := stack[len(stack)-1]
		f.lineContext[i] = LineContext{Kind: top.kind, TaskName: top.taskName}

		if m := keyLineRe.FindStringSubmatch(raw); m != nil {
			key := m[2]
			spacing := m[3]
			value := m[4]
			keyStart := len(m[1])
			keyEnd := keyStart + len(key)

			switch top.kind {
			case "root":
				switch key {
				case "tasks":
					f.TasksLine = i
					stack = append(stack, frame{indent: indent, kind: "tasks"})

				case "method":
					stack = append(stack, frame{indent: indent, kind: "method"})

				case "includes":
					stack = append(stack, frame{indent: indent, kind: "includes"})

				case "vars":
					stack = append(stack, frame{indent: indent, kind: "vars"})

				default:
					stack = append(stack, frame{indent: indent, kind: "other"})
				}

			case "tasks":
				if f.TaskIndent == 0 {
					f.TaskIndent = indent
				}

				if _, exists := f.Tasks[key]; exists {
					diags = append(diags, Diagnostic{
						Range:    Range{Start: Position{i, keyStart}, End: Position{i, keyEnd}},
						Severity: SeverityError,
						Code:     CodeDuplicateTask,
						Message:  "task \"" + key + "\" is already defined",
					})
				} else {
					f.Tasks[key] = &Task{
						Name:      key,
						NameRange: Range{Start: Position{i, keyStart}, End: Position{i, keyEnd}},
						DefLine:   i,
						EndLine:   i,
					}

					f.Order = append(f.Order, key)
				}

				stack = append(stack, frame{indent: indent, kind: "taskbody", taskName: key})

			case "method":
				stack = append(stack, frame{indent: indent, kind: "other"})

			case "vars":
				trimmedVal := strings.TrimSpace(value)
				if _, exists := f.Vars[key]; exists {
					diags = append(diags, Diagnostic{
						Range: Range{
							Start: Position{i, keyStart},
							End:   Position{i, keyEnd},
						},
						Severity: SeverityError,
						Code:     CodeDuplicateVar,
						Message:  fmt.Sprintf("var %q is already defined", key),
					})
				} else {
					v := &TaskVar{
						Name: key,
						NameRange: Range{
							Start: Position{i, keyStart},
							End:   Position{i, keyEnd},
						},
					}

					if trimmedVal != "" {
						v.Kind = TaskVarScalarKind
						v.Value = stripInlineComment(trimmedVal)
					}

					f.Vars[key] = v
				}

				if trimmedVal != "" {
					stack = append(stack, frame{indent: indent, kind: "other"})
				} else {
					stack = append(stack, frame{indent: indent, kind: "varbody", varName: key})
				}

			case "varbody":
				v := f.Vars[top.varName]
				switch key {
				case "sh":
					if v != nil {
						v.Kind = TaskVarShellKind
						v.Value = stripInlineComment(strings.TrimSpace(value))
					}

				case "ref":
					clean := stripInlineComment(strings.TrimSpace(value))
					name := strings.TrimPrefix(clean, ".")
					start := valueOffset(keyEnd, spacing)
					if strings.HasPrefix(clean, ".") {
						start++
					}

					if v != nil {
						v.Kind = TaskVarRefKind
						v.Value = name
					}

					if name != "" {
						f.Refs = append(f.Refs, Ref{
							Name:  name,
							Owner: top.varName,
							Kind:  RefVar,
							Range: Range{
								Start: Position{i, start},
								End:   Position{i, start + len(name)},
							},
						})
					}

				case "map":
					if v != nil {
						v.Kind = TaskVarMapKind
					}
				}

				stack = append(stack, frame{indent: indent, kind: "other"})

			case "taskbody":
				owner := top.taskName
				if f.BodyIndent == 0 {
					f.BodyIndent = indent
				}

				switch key {
				case "desc":
					if t, ok := f.Tasks[owner]; ok {
						t.Desc = strings.Trim(strings.TrimSpace(value), `"'`)
					}

					stack = append(stack, frame{indent: indent, kind: "other", taskName: owner})

				case "deps":
					if strings.TrimSpace(value) != "" {
						parseInlineRefs(raw, i, valueOffset(keyEnd, spacing), value, owner, RefDep, f)
					}

					stack = append(stack, frame{indent: indent, kind: "depslist", taskName: owner})

				case "cmds":
					stack = append(stack, frame{indent: indent, kind: "cmdslist", taskName: owner})

				default:
					stack = append(stack, frame{indent: indent, kind: "other", taskName: owner})
				}

			case "includes":
				f.Includes[key] = strings.TrimSpace(value)
				stack = append(stack, frame{indent: indent, kind: "other"})

			default:
				stack = append(stack, frame{indent: indent, kind: "other", taskName: top.taskName})
			}

			continue
		}

		if m := listItemRe.FindStringSubmatch(raw); m != nil {
			restStart := len(m[1]) + 1 + len(m[2])
			rest := m[3]
			switch top.kind {
			case "depslist":
				addListRef(rest, restStart, i, top.taskName, RefDep, f)

			case "cmdslist":
				addListRef(rest, restStart, i, top.taskName, RefCall, f)

			}

			continue
		}
	}

	for _, name := range f.Order {
		t := f.Tasks[name]
		end := len(lines) - 1
		for end > t.DefLine && strings.TrimSpace(lines[end]) == "" {
			end--
		}

		t.EndLine = end
	}

	for idx, name := range f.Order {
		if idx+1 >= len(f.Order) {
			continue
		}

		cur := f.Tasks[name]
		next := f.Tasks[f.Order[idx+1]]
		if next.DefLine-1 < cur.EndLine {
			cur.EndLine = next.DefLine - 1
		}
	}

	if f.TaskIndent == 0 {
		f.TaskIndent = 2
	}

	if f.BodyIndent == 0 {
		f.BodyIndent = f.TaskIndent + 2
	}

	for _, r := range f.Refs {
		if strings.Contains(r.Name, ":") {
			continue
		}

		if r.Kind == RefVar {
			if _, ok := f.Vars[r.Name]; !ok {
				diags = append(diags, Diagnostic{
					Range:    r.Range,
					Severity: SeverityWarning,
					Code:     CodeUndefinedVar,
					Message:  fmt.Sprintf("var %q not found", r.Name),
				})
			}

			continue
		}

		if _, ok := f.Tasks[r.Name]; !ok {
			diags = append(diags, Diagnostic{
				Range:    r.Range,
				Severity: SeverityWarning,
				Code:     CodeUndefinedTask,
				Message:  "task \"" + r.Name + "\" not found",
			})

			continue
		}

		if r.Kind == RefDep && r.Name == r.Owner {
			diags = append(diags, Diagnostic{
				Range:    r.Range,
				Severity: SeverityError,
				Code:     CodeSelfDependency,
				Message:  "task \"" + r.Owner + "\" cannot depend on itself",
			})
		}
	}

	return f, diags
}

func valueOffset(keyEnd int, spacing string) int {
	return keyEnd + 1 + len(spacing)
}

func stripInlineComment(s string) string {
	if idx := strings.Index(s, " #"); idx >= 0 {
		return s[:idx]
	}

	return s
}

func parseInlineRefs(raw string, lineIndex int, valueStart int, value string, owner string, kind RefKind, f *File) {
	inner := strings.TrimSpace(stripInlineComment(value))
	if strings.HasPrefix(inner, "[") && strings.HasSuffix(inner, "]") {
		inner = inner[1 : len(inner)-1]
	}

	searchFrom := valueStart
	for _, part := range strings.Split(inner, ",") {
		name := strings.Trim(strings.TrimSpace(part), `"'`)
		if name == "" {
			continue
		}

		idx := strings.Index(raw[searchFrom:], name)
		if idx < 0 {
			continue
		}

		start := searchFrom + idx
		end := start + len(name)
		f.Refs = append(f.Refs, Ref{
			Name:  name,
			Owner: owner,
			Kind:  kind,
			Range: Range{Start: Position{lineIndex, start}, End: Position{lineIndex, end}},
		})
		searchFrom = end
	}
}

func addListRef(rest string, restStart int, lineIndex int, owner string, kind RefKind, f *File) {
	rest = stripInlineComment(rest)
	trimmed := strings.TrimSpace(rest)
	if trimmed == "" {
		return
	}

	if kind == RefCall {
		loc := taskCallRe.FindStringSubmatchIndex(rest)
		if loc == nil {
			return
		}

		name := strings.Trim(rest[loc[2]:loc[3]], `"'`)
		start := restStart + loc[2]
		f.Refs = append(f.Refs, Ref{
			Name: name, Owner: owner, Kind: RefCall,
			Range: Range{Start: Position{lineIndex, start}, End: Position{lineIndex, start + len(name)}},
		})
		return
	}

	if strings.HasPrefix(trimmed, "task:") {
		sub := strings.TrimSpace(strings.TrimPrefix(trimmed, "task:"))
		idx := strings.Index(rest, sub)
		if idx < 0 || sub == "" {
			return
		}

		name := strings.Trim(sub, `"'`)
		start := restStart + idx
		f.Refs = append(f.Refs, Ref{
			Name: name, Owner: owner, Kind: RefCall,
			Range: Range{Start: Position{lineIndex, start}, End: Position{lineIndex, start + len(name)}},
		})
		return
	}

	name := strings.Trim(trimmed, `"'`)
	if name == "" {
		return
	}

	idx := strings.Index(rest, name)
	if idx < 0 {
		idx = 0
	}

	start := restStart + idx
	f.Refs = append(f.Refs, Ref{
		Name:  name,
		Owner: owner,
		Kind:  RefDep,
		Range: Range{Start: Position{lineIndex, start}, End: Position{lineIndex, start + len(name)}},
	})
}
