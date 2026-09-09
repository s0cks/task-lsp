package lsp

import (
	"context"
	"encoding/json"
	"fmt"
	"log"
	"os"
	"strings"

	"taskfile-lsp/internal/rpc"
	"taskfile-lsp/internal/taskfile"
)

type Server struct {
	docs *DocumentStore
	log  *log.Logger
}

func NewServer(logger *log.Logger) *Server {
	return &Server{docs: NewDocumentStore(), log: logger}
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

func (s *Server) handleInitialize(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p InitializeParams
	if len(params) > 0 {
		if err := json.Unmarshal(params, &p); err != nil {
			return nil, rpc.NewError(rpc.InvalidParams, err.Error())
		}
	}

	result := InitializeResult{
		Capabilities: ServerCapabilities{
			TextDocumentSync:       SyncFull,
			HoverProvider:          true,
			DocumentSymbolProvider: true,
			DefinitionProvider:     true,
			ReferencesProvider:     true,
			CodeActionProvider:     true,
			CompletionProvider: &CompletionOptions{
				TriggerCharacters: []string{":", "-"},
			},
		},
		ServerInfo: &ServerInfo{Name: "taskfile-lsp", Version: "0.1.0"},
	}

	return result, nil
}

func (s *Server) handleInitialized(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	s.log.Println("client initialized")
}

func (s *Server) handleShutdown(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	return nil, nil
}

func (s *Server) handleExit(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	os.Exit(0)
}

func (s *Server) handleDidOpen(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	var p DidOpenTextDocumentParams
	if err := json.Unmarshal(params, &p); err != nil {
		s.log.Printf("didOpen: %v", err)
		return
	}
	diags := s.docs.Open(&Document{
		URI:        p.TextDocument.URI,
		LanguageID: p.TextDocument.LanguageID,
		Version:    p.TextDocument.Version,
		Text:       p.TextDocument.Text,
	})
	s.publish(conn, p.TextDocument.URI, p.TextDocument.Version, diags)
}

func (s *Server) handleDidChange(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	var p DidChangeTextDocumentParams
	if err := json.Unmarshal(params, &p); err != nil {
		s.log.Printf("didChange: %v", err)
		return
	}
	if len(p.ContentChanges) == 0 {
		return
	}
	text := p.ContentChanges[len(p.ContentChanges)-1].Text
	diags := s.docs.Update(p.TextDocument.URI, p.TextDocument.Version, text)
	s.publish(conn, p.TextDocument.URI, p.TextDocument.Version, diags)
}

func (s *Server) handleDidClose(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	var p DidCloseTextDocumentParams
	if err := json.Unmarshal(params, &p); err != nil {
		return
	}
	s.docs.Close(p.TextDocument.URI)
}

func (s *Server) handleDidSave(ctx context.Context, conn *rpc.Conn, params json.RawMessage) {
	var p DidSaveTextDocumentParams
	if err := json.Unmarshal(params, &p); err != nil {
		return
	}
	if doc, ok := s.docs.Get(p.TextDocument.URI); ok {
		_, diags := taskfile.Parse(doc.Text)
		s.publish(conn, doc.URI, doc.Version, diags)
	}
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
	conn.Notify("textDocument/publishDiagnostics", PublishDiagnosticsParams{
		URI:         uri,
		Version:     &v,
		Diagnostics: diags,
	})
}

func (s *Server) handleHover(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p TextDocumentPositionParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}
	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok {
		return nil, nil
	}
	if doc.Parsed != nil {
		if name, ok := doc.Parsed.NameOrRefAt(toTFPos(p.Position)); ok {
			if t, ok := doc.Parsed.Tasks[name]; ok {
				value := fmt.Sprintf("**task: %s**", t.Name)
				if t.Desc != "" {
					value += "\n\n" + t.Desc
				}
				return Hover{Contents: MarkupContent{Kind: MarkupMarkdown, Value: value}}, nil
			}
		}
	}
	word := wordAt(doc.Text, p.Position)
	if word == "" {
		return nil, nil
	}
	return Hover{Contents: MarkupContent{Kind: MarkupMarkdown, Value: fmt.Sprintf("`%s`", word)}}, nil
}

func (s *Server) handleDefinition(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p TextDocumentPositionParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}
	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok || doc.Parsed == nil {
		return nil, nil
	}
	name, ok := doc.Parsed.NameOrRefAt(toTFPos(p.Position))
	if !ok {
		return nil, nil
	}
	task, ok := doc.Parsed.Tasks[name]
	if !ok {
		return nil, nil
	}
	return Location{URI: p.TextDocument.URI, Range: toRange(task.NameRange)}, nil
}

func (s *Server) handleReferences(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p ReferenceParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}
	locs := []Location{}
	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok || doc.Parsed == nil {
		return locs, nil
	}
	name, ok := doc.Parsed.NameOrRefAt(toTFPos(p.Position))
	if !ok {
		return locs, nil
	}
	if p.Context.IncludeDeclaration {
		if t, ok := doc.Parsed.Tasks[name]; ok {
			locs = append(locs, Location{URI: p.TextDocument.URI, Range: toRange(t.NameRange)})
		}
	}
	for _, r := range doc.Parsed.Refs {
		if r.Name == name {
			locs = append(locs, Location{URI: p.TextDocument.URI, Range: toRange(r.Range)})
		}
	}
	return locs, nil
}

var rootKeywords = []CompletionItem{
	{Label: "version", Kind: CompletionItemKeyword},
	{Label: "tasks", Kind: CompletionItemKeyword},
	{Label: "includes", Kind: CompletionItemKeyword},
	{Label: "vars", Kind: CompletionItemKeyword},
	{Label: "env", Kind: CompletionItemKeyword},
	{Label: "silent", Kind: CompletionItemKeyword},
}

var taskBodyKeywords = []CompletionItem{
	{Label: "cmds", Kind: CompletionItemKeyword, Detail: "Command list for a task"},
	{Label: "deps", Kind: CompletionItemKeyword, Detail: "Task dependencies"},
	{Label: "vars", Kind: CompletionItemKeyword, Detail: "Task-local variables"},
	{Label: "env", Kind: CompletionItemKeyword, Detail: "Environment variables"},
	{Label: "desc", Kind: CompletionItemKeyword, Detail: "Task description"},
	{Label: "dir", Kind: CompletionItemKeyword, Detail: "Working directory"},
	{Label: "sources", Kind: CompletionItemKeyword, Detail: "Inputs for checksum/timestamp checks"},
	{Label: "generates", Kind: CompletionItemKeyword, Detail: "Outputs produced by this task"},
	{Label: "status", Kind: CompletionItemKeyword, Detail: "Commands that decide if this task is up to date"},
	{Label: "preconditions", Kind: CompletionItemKeyword, Detail: "Conditions checked before running"},
	{Label: "silent", Kind: CompletionItemKeyword, Detail: "Suppress command echo"},
}

func taskNameItems(f *taskfile.File) []CompletionItem {
	items := make([]CompletionItem, 0, len(f.Order))
	for _, name := range f.Order {
		items = append(items, CompletionItem{Label: name, Kind: CompletionItemValue, Detail: f.Tasks[name].Desc})
	}
	return items
}

func (s *Server) handleCompletion(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p CompletionParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}
	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok || doc.Parsed == nil {
		return CompletionList{Items: taskBodyKeywords}, nil
	}
	lc := doc.Parsed.ContextAt(toTFPos(p.Position))
	switch lc.Kind {
	case "depslist":
		return CompletionList{Items: taskNameItems(doc.Parsed)}, nil
	case "cmdslist":
		line := lineAt(doc.Text, p.Position.Line)
		cursor := min(p.Position.Character, len(line))
		if strings.Contains(line[:cursor], "task:") {
			return CompletionList{Items: taskNameItems(doc.Parsed)}, nil
		}
		return CompletionList{Items: []CompletionItem{}}, nil
	case "tasks":
		return CompletionList{Items: []CompletionItem{}}, nil
	case "root":
		return CompletionList{Items: rootKeywords}, nil
	default:
		return CompletionList{Items: taskBodyKeywords}, nil
	}
}

func (s *Server) handleSymbols(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p SymbolsParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}

	symbols := []Symbol{}
	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok || doc.Parsed == nil {
		return symbols, nil
	}

	for _, task := range doc.Parsed.Tasks {
		symbols = append(symbols, Symbol{
			Name:   task.Name,
			Detail: "function",
			Kind:   12,
			SelectionRange: Range{
				Start: Position{
					Line:      task.NameRange.Start.Line,
					Character: task.NameRange.Start.Character,
				},
				End: Position{
					Line:      task.NameRange.Start.Line,
					Character: task.NameRange.Start.Character,
				},
			},
			Range: Range{
				Start: Position{
					Line:      task.NameRange.Start.Line,
					Character: task.NameRange.Start.Character,
				},
				End: Position{
					Line:      task.NameRange.Start.Line,
					Character: task.NameRange.Start.Character,
				},
			},
		})
	}

	return symbols, nil
}

func (s *Server) handleCodeAction(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p CodeActionParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}

	actions := []CodeAction{}
	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok || doc.Parsed == nil {
		return actions, nil
	}

	for _, d := range p.Context.Diagnostics {
		if d.Code != taskfile.CodeUndefinedTask {
			continue
		}

		name := strings.TrimSpace(textInRange(doc.Text, d.Range))
		if name == "" {
			continue
		}

		actions = append(actions, CodeAction{
			Title:       fmt.Sprintf("Create task %q", name),
			Kind:        "quickfix",
			Diagnostics: []Diagnostic{d},
			Edit: &WorkspaceEdit{
				Changes: map[string][]TextEdit{p.TextDocument.URI: {newTaskEdit(doc.Parsed, name)}},
			},
		})
	}

	if task, ok := doc.Parsed.TaskAt(toTFPos(p.Range.Start)); ok && task.Desc == "" {
		at := Position{Line: task.DefLine + 1, Character: 0}
		actions = append(actions, CodeAction{
			Title: "Add description",
			Kind:  "refactor",
			Edit: &WorkspaceEdit{
				Changes: map[string][]TextEdit{p.TextDocument.URI: {{
					Range:   Range{Start: at, End: at},
					NewText: strings.Repeat(" ", doc.Parsed.BodyIndent) + "desc: \"\"\n",
				}}},
			},
		})
	}

	return actions, nil
}

func newTaskEdit(f *taskfile.File, name string) TextEdit {
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
