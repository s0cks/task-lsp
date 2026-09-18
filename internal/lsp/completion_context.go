package lsp

import "strings"

type completionContext int

const (
	ctxUnknown completionContext = iota
	ctxRoot
	ctxTaskBody
	ctxVarBody
	ctxDepsList
	ctxCmdsList
	ctxCmdsTaskField
	ctxMethod
	ctxTemplateVar
)

func indentOf(line string) int {
	n := 0
	for n < len(line) && line[n] == ' ' {
		n++
	}
	return n
}

func lineKeyName(trimmed string) string {
	before, _, ok := strings.Cut(trimmed, ":")
	if !ok {
		return ""
	}
	key := strings.TrimSpace(before)
	if key == "" || strings.HasPrefix(key, "-") || strings.HasPrefix(key, "#") {
		return ""
	}
	return key
}

func nearestEnclosingKeyAbove(text string, line, threshold int) (key string, indent int, ok bool) {
	lines := strings.Split(text, "\n")
	if line < 0 || line >= len(lines) {
		return "", 0, false
	}

	for i := line - 1; i >= 0; i-- {
		raw := lines[i]
		trimmed := strings.TrimLeft(raw, " ")
		if trimmed == "" {
			continue
		}

		ind := indentOf(raw)
		if ind >= threshold {
			continue
		}

		if strings.HasPrefix(trimmed, "- ") || trimmed == "-" {
			threshold = ind
			continue
		}

		if k := lineKeyName(trimmed); k != "" {
			return k, ind, true
		}

		threshold = ind
	}

	return "", 0, false
}

func nearestEnclosingKey(text string, line, prefixLen int) (key string, indent int, ok bool) {
	lines := strings.Split(text, "\n")
	if line < 0 || line >= len(lines) {
		return "", 0, false
	}

	cur := lines[line]
	threshold := prefixLen
	if trimmed := strings.TrimLeft(cur, " "); trimmed != "" {
		threshold = indentOf(cur)
	}

	return nearestEnclosingKeyAbove(text, line, threshold)
}

func templateVarPrefix(prefix string) (string, bool) {
	i := strings.LastIndex(prefix, "{{")
	if i < 0 {
		return "", false
	}

	inside := prefix[i+2:]
	if strings.Contains(inside, "}}") {
		return "", false
	}

	inside = strings.TrimLeft(inside, " ")
	if !strings.HasPrefix(inside, ".") {
		return "", false
	}

	return strings.TrimLeft(inside[1:], " "), true
}

func wordPrefix(line string) string {
	i := len(line)
	for i > 0 {
		c := line[i-1]
		if (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == ':' {
			i--
			continue
		}

		break
	}

	return line[i:]
}

func classifyCompletion(text string, pos Position) (ctx completionContext, partial string) {
	line := lineAt(text, pos.Line)
	runes := []rune(line)
	cursor := min(pos.Character, len(runes))
	prefix := string(runes[:cursor])

	if tv, ok := templateVarPrefix(prefix); ok {
		return ctxTemplateVar, tv
	}

	trimmedLine := strings.TrimLeft(line, " ")
	if k := lineKeyName(trimmedLine); k != "" {
		switch k {
		case "deps":
			return ctxDepsList, wordPrefix(prefix)

		case "cmds":
			return ctxCmdsList, wordPrefix(prefix)

		case "method":
			return ctxMethod, wordPrefix(prefix)

		}
	}

	if strings.Contains(prefix, "task:") {
		if key, _, ok := nearestEnclosingKey(text, pos.Line, cursor); ok && key == "cmds" {
			return ctxCmdsTaskField, wordPrefix(prefix)
		}
	}

	key, indent, ok := nearestEnclosingKey(text, pos.Line, cursor)
	if !ok {
		return ctxRoot, ""
	}

	switch key {
	case "deps":
		return ctxDepsList, wordPrefix(prefix)

	case "cmds":
		return ctxCmdsList, wordPrefix(prefix)

	case "method":
		return ctxMethod, wordPrefix(prefix)

	case "vars", "env":
		return ctxVarBody, wordPrefix(prefix)

	case "tasks":
		return ctxRoot, ""

	}

	if parentKey, _, ok := nearestEnclosingKeyAbove(text, pos.Line, indent); ok {
		switch parentKey {
		case "tasks":
			return ctxTaskBody, wordPrefix(prefix)

		case "vars", "env":
			return ctxVarBody, wordPrefix(prefix)

		}
	}

	if indent > 0 {
		return ctxTaskBody, wordPrefix(prefix)
	}

	return ctxRoot, ""
}
