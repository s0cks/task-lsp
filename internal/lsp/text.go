package lsp

import "strings"

func lineAt(text string, line int) string {
	lines := strings.Split(text, "\n")
	if line < 0 || line >= len(lines) {
		return ""
	}
	return lines[line]
}

func wordAt(text string, pos Position) string {
	runes := []rune(lineAt(text, pos.Line))
	if pos.Character < 0 || pos.Character > len(runes) {
		return ""
	}
	isWordChar := func(r rune) bool {
		return (r >= 'a' && r <= 'z') || (r >= 'A' && r <= 'Z') || (r >= '0' && r <= '9') || r == '_' || r == '-'
	}
	start := pos.Character
	for start > 0 && isWordChar(runes[start-1]) {
		start--
	}
	end := pos.Character
	for end < len(runes) && isWordChar(runes[end]) {
		end++
	}
	if start == end {
		return ""
	}
	return string(runes[start:end])
}

func textInRange(text string, r Range) string {
	if r.Start.Line != r.End.Line {
		return ""
	}
	runes := []rune(lineAt(text, r.Start.Line))
	start, end := r.Start.Character, r.End.Character
	if start < 0 || end > len(runes) || start > end {
		return ""
	}
	return string(runes[start:end])
}
