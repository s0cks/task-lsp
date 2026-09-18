package lsp

import "testing"

func TestClassifyCompletion(t *testing.T) {
	tests := []struct {
		name     string
		text     string
		line     int
		char     int
		wantCtx  completionContext
		wantPart string
	}{
		{
			name:    "blank line under block deps list",
			text:    "tasks:\n  build:\n    deps:\n      - \n",
			line:    3,
			char:    8,
			wantCtx: ctxDepsList,
		},
		{
			name:     "inline flow deps list on the deps line itself",
			text:     "tasks:\n  build:\n    deps: [a, ]\n",
			line:     2,
			char:     12,
			wantCtx:  ctxDepsList,
			wantPart: "a",
		},
		{
			name:    "task field inside a cmds list entry",
			text:    "tasks:\n  build:\n    cmds:\n      - task: \n",
			line:    3,
			char:    14,
			wantCtx: ctxCmdsTaskField,
		},
		{
			name:    "bare cmds list entry (no task field)",
			text:    "tasks:\n  build:\n    cmds:\n      - \n",
			line:    3,
			char:    8,
			wantCtx: ctxCmdsList,
		},
		{
			name:     "template var interpolation",
			text:     "tasks:\n  build:\n    cmds:\n      - echo {{.FOO\n",
			line:     3,
			char:     19,
			wantCtx:  ctxTemplateVar,
			wantPart: "FOO",
		},
		{
			name:    "closed template var does not trigger",
			text:    "tasks:\n  build:\n    cmds:\n      - echo {{.FOO}} bar\n",
			line:    3,
			char:    22,
			wantCtx: ctxCmdsList,
		},
		{
			name:    "task body, not confused with var body",
			text:    "tasks:\n  build:\n    \n",
			line:    2,
			char:    4,
			wantCtx: ctxTaskBody,
		},
		{
			name:    "var body, not confused with task body",
			text:    "vars:\n  GREETING:\n    \n",
			line:    2,
			char:    4,
			wantCtx: ctxVarBody,
		},
		{
			name:    "task-local var body still recognized as var body",
			text:    "tasks:\n  build:\n    vars:\n      GREETING:\n        \n",
			line:    4,
			char:    8,
			wantCtx: ctxVarBody,
		},
		{
			name:    "root level",
			text:    "version: '3'\n\n",
			line:    1,
			char:    0,
			wantCtx: ctxRoot,
		},
		{
			name:    "method field",
			text:    "tasks:\n  build:\n    method: \n",
			line:    2,
			char:    12,
			wantCtx: ctxMethod,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			ctx, part := classifyCompletion(tt.text, Position{Line: tt.line, Character: tt.char})
			if ctx != tt.wantCtx {
				t.Errorf("ctx = %v, want %v (partial=%q)", ctx, tt.wantCtx, part)
			}

			if tt.wantPart != "" && part != tt.wantPart {
				t.Errorf("partial = %q, want %q", part, tt.wantPart)
			}
		})
	}
}
