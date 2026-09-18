package lsp

import (
	"context"
	"encoding/json"
	"strings"

	"taskfile-lsp/internal/document"
	"taskfile-lsp/internal/rpc"
)

var methodKeywords = []CompletionItem{
	{Label: "checksum", Kind: CompletionItemKeyword},
	{Label: "timestamp", Kind: CompletionItemKeyword},
	{Label: "none", Kind: CompletionItemKeyword},
}

var varBodyKeywords = []CompletionItem{
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
	{Label: "method", Kind: CompletionItemKeyword, Detail: "How up-to-date checks are performed"},
}

var builtinTemplateVars = []CompletionItem{
	{Label: "TASK", Kind: CompletionItemValue, Detail: "The name of the current task"},
	{Label: "ROOT_DIR", Kind: CompletionItemValue, Detail: "The root Taskfile's directory"},
	{Label: "TASKFILE_DIR", Kind: CompletionItemValue, Detail: "The directory of the Taskfile this task is defined in"},
	{Label: "USER_WORKING_DIR", Kind: CompletionItemValue, Detail: "The directory task was run from"},
	{Label: "CLI_ARGS", Kind: CompletionItemValue, Detail: "Extra CLI arguments passed after --"},
}

func (s *Server) taskCompletionItems(uri string) []CompletionItem {
	var items []CompletionItem

	f, ok := s.ws.File(uri)
	if !ok || f.Parsed == nil {
		return items
	}

	f.Parsed.VisitTasks(func(_ uint64, t document.Task) bool {
		items = append(items, CompletionItem{
			Label: t.Name(), Kind: CompletionItemValue, Detail: t.Desc(),
		})
		return true
	})

	for _, e := range f.IncludeEdges {
		if e.TargetURI == "" {
			continue
		}

		target, ok := s.ws.File(e.TargetURI)
		if !ok || target.Parsed == nil {
			continue
		}

		target.Parsed.VisitTasks(func(_ uint64, t document.Task) bool {
			items = append(items, CompletionItem{
				Label: e.Namespace + ":" + t.Name(), Kind: CompletionItemValue, Detail: t.Desc(),
			})

			return true
		})
	}

	return items
}

func (s *Server) varCompletionItems(uri string) []CompletionItem {
	items := append([]CompletionItem{}, builtinTemplateVars...)

	f, ok := s.ws.File(uri)
	if !ok || f.Parsed == nil {
		return items
	}

	f.Parsed.VisitVars(func(_ uint64, v document.Var) bool {
		items = append(items, CompletionItem{Label: v.Name(), Kind: CompletionItemValue})
		return true
	})

	return items
}

func filterByPrefix(items []CompletionItem, prefix string) []CompletionItem {
	if prefix == "" {
		return items
	}

	out := items[:0:0]
	for _, it := range items {
		if strings.HasPrefix(it.Label, prefix) {
			out = append(out, it)
		}
	}

	return out
}

func (s *Server) handleCompletion(ctx context.Context, conn *rpc.Conn, params json.RawMessage) (any, *rpc.Error) {
	var p CompletionParams
	if err := json.Unmarshal(params, &p); err != nil {
		return nil, rpc.NewError(rpc.InvalidParams, err.Error())
	}

	doc, ok := s.docs.Get(p.TextDocument.URI)
	if !ok {
		return CompletionList{Items: rootKeywords}, nil
	}

	completionCtx, partial := classifyCompletion(doc.Text, p.Position)

	switch completionCtx {
	case ctxDepsList, ctxCmdsTaskField:
		return CompletionList{Items: filterByPrefix(s.taskCompletionItems(p.TextDocument.URI), partial)}, nil

	case ctxTemplateVar:
		return CompletionList{Items: filterByPrefix(s.varCompletionItems(p.TextDocument.URI), partial)}, nil

	case ctxCmdsList:
		return CompletionList{Items: []CompletionItem{}}, nil

	case ctxVarBody:
		return CompletionList{Items: varBodyKeywords}, nil

	case ctxTaskBody:
		return CompletionList{Items: taskBodyKeywords}, nil

	case ctxMethod:
		return CompletionList{Items: methodKeywords}, nil

	case ctxRoot:
		return CompletionList{Items: rootKeywords}, nil

	default:
		return CompletionList{Items: []CompletionItem{}}, nil

	}
}
