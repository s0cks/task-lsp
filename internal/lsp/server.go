package lsp

import (
	"fmt"
	"log"
	"strings"

	"taskfile-lsp/internal/rpc"
	"taskfile-lsp/internal/taskfile"
)

type Server struct {
	docs *DocumentStore
	log  *log.Logger
}

func NewServer(logger *log.Logger) *Server {
	return &Server{docs: NewDocumentStore(logger), log: logger}
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
	conn.HandleRequest("textDocument/documentSymbol", s.handleSymbols)
}

func toRange(r taskfile.Range) Range {
	return Range{
		Start: Position{Line: r.Start.Line, Character: r.Start.Character},
		End:   Position{Line: r.End.Line, Character: r.End.Character},
	}
}

func toTFPos(p Position) taskfile.Position {
	return taskfile.Position{Line: p.Line, Character: p.Character}
}

func (s *Server) publish(conn *rpc.Conn, uri string, version int, raw []taskfile.Diagnostic) {
	diags := make([]Diagnostic, 0, len(raw))
	for _, d := range raw {
		diags = append(diags, Diagnostic{
			Range:    toRange(d.Range),
			Severity: DiagnosticSeverity(d.Severity),
			Code:     d.Code,
			Source:   "taskfile-lsp",
			Message:  d.Message,
		})
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

func NewTaskEdit(f *taskfile.File, name string) TextEdit {
	indent := strings.Repeat(" ", f.TaskIndent)
	body := strings.Repeat(" ", f.BodyIndent)
	cmdIndent := strings.Repeat(" ", f.BodyIndent+2)
	text := fmt.Sprintf("\n%s%s:\n%scmds:\n%s- echo \"TODO: implement %s\"\n", indent, name, body, cmdIndent, name)

	insertLine := f.TasksLine + 1
	if len(f.Order) > 0 {
		insertLine = f.Tasks[f.Order[len(f.Order)-1]].EndLine + 1
	}
	at := Position{Line: insertLine, Character: 0}
	return TextEdit{Range: Range{Start: at, End: at}, NewText: text}
}
