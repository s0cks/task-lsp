package lsp

import (
	"context"
	"encoding/json"
	"strings"
	"taskfile-lsp/internal/rpc"
	"taskfile-lsp/internal/taskfile"
)

var methodKeywords = []CompletionItem{
	{Label: "checksum", Kind: CompletionItemKeyword, Detail: ""},
	{Label: "timestamp", Kind: CompletionItemKeyword, Detail: ""},
	{Label: "none", Kind: CompletionItemKeyword, Detail: ""},
}

var varBodyKeywords = []CompletionItem{
	{Label: "scalar", Kind: CompletionItemKeyword, Detail: "A scalar value"},
	{Label: "sh", Kind: CompletionItemKeyword, Detail: "A shell command value"},
	{Label: "ref", Kind: CompletionItemKeyword, Detail: "A reference to another variable"},
	{Label: "map", Kind: CompletionItemKeyword, Detail: "A nested map value"},
}

var rootKeywords = []CompletionItem{
	{Label: "method", Kind: CompletionItemKeyword},
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
	{Label: "dotenv", Kind: CompletionItemKeyword, Detail: "Load environment variables from .env files"},
	{Label: "summary", Kind: CompletionItemKeyword, Detail: "Detailed description shown in --summary"},
	{Label: "prompt", Kind: CompletionItemKeyword, Detail: "Prompts shown before task execution"},
	{Label: "aliases", Kind: CompletionItemKeyword, Detail: "Alternative names for the task"},
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

	case "tasks", "vars":
		return CompletionList{Items: []CompletionItem{}}, nil

	case "root":
		return CompletionList{Items: rootKeywords}, nil

	case "taskbody":
		return CompletionList{Items: taskBodyKeywords}, nil

	case "varbody":
		return CompletionList{Items: varBodyKeywords}, nil

	case "method":
		return CompletionList{Items: methodKeywords}, nil

	default:
		return CompletionList{Items: []CompletionItem{}}, nil
	}
}
