package lsp

import (
	"context"
	"encoding/json"
	"strings"
	"testing"
)

func newTestServer() *Server {
	return NewServer()
}

func openDoc(t *testing.T, s *Server, uri, text string) {
	t.Helper()
	s.docs.Open(&Document{URI: uri, LanguageID: "yaml", Version: 1, Text: text})
}

func codeActionsAt(t *testing.T, s *Server, uri string, r Range) []CodeAction {
	t.Helper()

	params, err := json.Marshal(CodeActionParams{
		TextDocument: TextDocumentIdentifier{URI: uri},
		Range:        r,
	})
	if err != nil {
		t.Fatalf("marshal params: %v", err)
	}

	result, rpcErr := s.handleCodeAction(context.Background(), nil, params)
	if rpcErr != nil {
		t.Fatalf("handleCodeAction: %v", rpcErr)
	}

	actions, ok := result.([]CodeAction)
	if !ok {
		t.Fatalf("unexpected result type %T", result)
	}

	return actions
}

func applyEdit(text string, edit TextEdit) string {
	lines := strings.Split(text, "\n")

	before := strings.Join(lines[:edit.Range.Start.Line], "\n")
	if edit.Range.Start.Line > 0 {
		before += "\n"
	}

	startLine := []rune(lines[edit.Range.Start.Line])
	prefix := string(startLine[:min(edit.Range.Start.Character, len(startLine))])

	endLine := []rune(lines[edit.Range.End.Line])
	suffix := string(endLine[min(edit.Range.End.Character, len(endLine)):])
	after := suffix
	if edit.Range.End.Line+1 < len(lines) {
		after += "\n" + strings.Join(lines[edit.Range.End.Line+1:], "\n")
	}

	return before + prefix + edit.NewText + after
}

func TestHandleCodeAction(t *testing.T) {
	const uri = "file:///Taskfile.yaml"

	tests := []struct {
		name       string
		text       string
		queryRange Range
		wantTitles []string
	}{
		{
			name:       "incomplete command offers a removal quickfix",
			text:       "tasks:\n  a:\n    cmds:\n      - echo a\n  b:\n    cmds:\n      -\n",
			queryRange: Range{Start: Position{Line: 6, Character: 0}, End: Position{Line: 6, Character: 7}},
			wantTitles: []string{`Remove: task "b" has an incomplete command`},
		},
		{
			name:       "incomplete dependency offers a removal quickfix",
			text:       "tasks:\n  a:\n    deps:\n      -\n",
			queryRange: Range{Start: Position{Line: 3, Character: 0}, End: Position{Line: 3, Character: 7}},
			wantTitles: []string{`Remove: task "a" has an incomplete dependency`},
		},
		{
			name:       "well-formed document offers nothing",
			text:       "tasks:\n  a:\n    cmds:\n      - echo a\n",
			queryRange: Range{Start: Position{Line: 1, Character: 0}, End: Position{Line: 1, Character: 3}},
			wantTitles: nil,
		},
		{
			name:       "cursor away from the diagnostic offers nothing",
			text:       "tasks:\n  a:\n    cmds:\n      - echo a\n  b:\n    cmds:\n      -\n",
			queryRange: Range{Start: Position{Line: 1, Character: 0}, End: Position{Line: 1, Character: 3}},
			wantTitles: nil,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			s := newTestServer()
			openDoc(t, s, uri, tt.text)
			defer s.docs.Close(uri)

			actions := codeActionsAt(t, s, uri, tt.queryRange)

			if len(actions) != len(tt.wantTitles) {
				t.Fatalf("got %d actions, want %d (%+v)", len(actions), len(tt.wantTitles), actions)
			}

			for i, want := range tt.wantTitles {
				if actions[i].Title != want {
					t.Errorf("action[%d].Title = %q, want %q", i, actions[i].Title, want)
				}
			}
		})
	}
}

func TestRemoveIncompleteNodeCodeAction_FixResolvesDiagnostic(t *testing.T) {
	const uri = "file:///Taskfile.yaml"
	text := "tasks:\n  a:\n    cmds:\n      - echo a\n  b:\n    cmds:\n      -\n"

	s := newTestServer()
	openDoc(t, s, uri, text)
	defer s.docs.Close(uri)

	diagsBefore := s.computeDiagnosticsFor(uri)
	if len(diagsBefore) == 0 {
		t.Fatal("expected the malformed input to produce a diagnostic")
	}

	actions := codeActionsAt(t, s, uri, Range{
		Start: Position{Line: 6, Character: 0},
		End:   Position{Line: 6, Character: 7},
	})

	if len(actions) != 1 {
		t.Fatalf("got %d actions, want 1: %+v", len(actions), actions)
	}

	edits := actions[0].Edit.Changes[uri]
	if len(edits) != 1 {
		t.Fatalf("got %d edits, want 1", len(edits))
	}

	fixed := applyEdit(text, edits[0])
	affected := s.docs.Update(uri, 2, fixed)
	for _, u := range affected {
		s.diagCache[u] = s.computeDiagnosticsFor(u)
	}

	diagsAfter := s.diagCache[uri]
	if len(diagsAfter) != 0 {
		t.Fatalf("diagnostics remain after applying the fix:\n%s\ngot: %+v", fixed, diagsAfter)
	}
}
