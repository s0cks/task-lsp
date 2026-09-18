package lsp

import (
	"fmt"

	"taskfile-lsp/internal/document"
	"taskfile-lsp/internal/rpc"
	"taskfile-lsp/internal/workspace"
)

type Server struct {
	ws        *workspace.Workspace
	docs      *DocumentStore
	diagCache map[string][]Diagnostic
}

func NewServer() *Server {
	ws := workspace.New()
	return &Server{
		ws:        ws,
		docs:      NewDocumentStore(ws),
		diagCache: make(map[string][]Diagnostic),
	}
}

func (s *Server) recomputeAndPublish(conn *rpc.Conn, affected []string) {
	for _, uri := range affected {
		diags := s.computeDiagnosticsFor(uri)
		s.diagCache[uri] = diags

		version := 0
		if f, ok := s.ws.File(uri); ok {
			version = f.Version
		}

		s.publish(conn, uri, version, diags)
	}
}

func (s *Server) Register(conn *rpc.Conn) {
	conn.HandleRequest("initialize", s.handleInitialize)
	conn.HandleNotification("initialized", s.handleInitialized)
	conn.HandleRequest("shutdown", s.handleShutdown)
	conn.HandleNotification("exit", s.handleExit)

	conn.HandleNotification("textDocument/didOpen", s.handleDidOpen)
	conn.HandleNotification("textDocument/didChange", s.handleDidChange)
	conn.HandleNotification("textDocument/didClose", s.handleDidClose)
	conn.HandleNotification("textDocument/didSave", s.handleDidSave)

	conn.HandleRequest("textDocument/hover", s.handleHover)
	conn.HandleRequest("textDocument/definition", s.handleDefinition)
	conn.HandleRequest("textDocument/references", s.handleReferences)
	conn.HandleRequest("textDocument/completion", s.handleCompletion)
	conn.HandleRequest("textDocument/codeAction", s.handleCodeAction)
	conn.HandleRequest("textDocument/documentSymbol", s.handleDocumentSymbols)

	conn.HandleRequest("workspace/symbol", s.handleWorkspaceSymbols)
}

func toRange(r document.Range) Range {
	return Range{
		Start: Position{Line: r.Start.Row, Character: r.Start.Col},
		End:   Position{Line: r.End.Row, Character: r.End.Col},
	}
}

func (s *Server) publish(conn *rpc.Conn, uri string, version int, diags []Diagnostic) {
	if diags == nil {
		diags = []Diagnostic{}
	}

	for i := range diags {
		if diags[i].Source == "" {
			diags[i].Source = "taskfile-lsp"
		}
	}

	v := version
	if err := conn.Notify("textDocument/publishDiagnostics", PublishDiagnosticsParams{
		URI:         uri,
		Version:     &v,
		Diagnostics: diags,
	}); err != nil {
		fmt.Printf("notify failed: %v", err)
	}
}

// func NewTaskEdit(f *taskfile.File, name string) TextEdit {
// 	indent := strings.Repeat(" ", f.TaskIndent)
// 	body := strings.Repeat(" ", f.BodyIndent)
// 	cmdIndent := strings.Repeat(" ", f.BodyIndent+2)
// 	text := fmt.Sprintf("\n%s%s:\n%scmds:\n%s- echo \"TODO: implement %s\"\n", indent, name, body, cmdIndent, name)
//
// 	insertLine := f.TasksLine + 1
// 	if len(f.Order) > 0 {
// 		insertLine = f.Tasks[f.Order[len(f.Order)-1]].EndLine + 1
// 	}
// 	at := Position{Line: insertLine, Character: 0}
// 	return TextEdit{Range: Range{Start: at, End: at}, NewText: text}
// }
