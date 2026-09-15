#include "document.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "seq.h"
#include "taskfile_parser.h"

Document* NewDocument(const char* path) {
  Document* doc = (Document*)calloc(1, sizeof(Document));
  if (!doc)
    return NULL;

  doc->kind = kDocumentKind;
  doc->path = path ? strdup(path) : NULL;
  return doc;
}

TaskNode* NewTaskNode(void) {
  TaskNode* task = (TaskNode*)calloc(1, sizeof(TaskNode));
  if (task)
    task->kind = kTaskKind;
  return task;
}

VarNode* NewVarNode(void) {
  VarNode* var = (VarNode*)calloc(1, sizeof(VarNode));
  if (var)
    var->kind = kVarKind;
  return var;
}

CommandNode* NewCommandNode(void) {
  CommandNode* cmd = (CommandNode*)calloc(1, sizeof(CommandNode));
  if (cmd) {
    cmd->kind = kCommandKind;
    cmd->cmd.kind = kStringKind;
  }
  return cmd;
}

CommentNode* NewCommentNode(void) {
  CommentNode* comment = (CommentNode*)calloc(1, sizeof(CommentNode));
  if (comment)
    comment->kind = kCommentKind;
  return comment;
}

ForNode* NewForNode(void) {
  ForNode* node = (ForNode*)calloc(1, sizeof(ForNode));
  if (node)
    node->kind = kForKind;
  return node;
}

DeferNode* NewDeferNode(void) {
  DeferNode* node = (DeferNode*)calloc(1, sizeof(DeferNode));
  if (node)
    node->kind = kDeferKind;
  return node;
}

OutputNode* NewOutputNode(void) {
  OutputNode* node = (OutputNode*)calloc(1, sizeof(OutputNode));
  if (node) {
    node->kind = kOutputKind;
    node->output_kind = kDefaultOutputKind;
  }
  return node;
}

MapNode* NewMapNode(void) {
  MapNode* map = (MapNode*)calloc(1, sizeof(MapNode));
  if (map)
    map->kind = kMapKind;
  return map;
}

void AddCommentToNode(DocumentNode* node, const CommentNode comment) {
  if (!node)
    return;
  CommentNode* slot = AppendNewCommentInSeq(&node->comments);
  *slot = comment;
}

bool IsFragmentDocument(Document* rhs) {
  return rhs && rhs->fragment;
}

bool IsPartialDocument(Document* rhs) {
  return rhs && rhs->partial;
}

char* GetDocumentPath(Document* rhs) {
  return rhs ? rhs->path : NULL;
}

StrView GetDocumentVersion(Document* rhs) {
  return rhs ? rhs->version : (StrView){0};
}

OutputNode* GetDocumentOutput(Document* rhs) {
  return rhs ? rhs->output : NULL;
}

NewlineStyle GetDocumentNewlineStyle(Document* rhs) {
  return rhs ? rhs->newline_style : kUnknownNewline;
}

int GetDocumentIndentWidth(Document* rhs) {
  return rhs && rhs->indent_width > 0 ? rhs->indent_width : 2;
}

bool DocumentUsesTabsForIndent(Document* rhs) {
  return rhs && rhs->indent_tabs;
}

uint64_t GetNumberOfBlankLinesInDocument(Document* rhs) {
  return GetNumberOfLinesInSeq(rhs ? &rhs->blank_lines : NULL);
}

int GetDocumentBlankLineAt(Document* doc, const uint64_t idx) {
  return doc ? GetLineInSeqAt(&doc->blank_lines, idx) : -1;
}

bool IsDocumentLineBlank(Document* doc, const int line) {
  if (!doc)
    return false;
  for (uint64_t i = 0; i < GetNumberOfLinesInSeq(&doc->blank_lines); i++) {
    if (GetLineInSeqAt(&doc->blank_lines, i) == line)
      return true;
  }
  return false;
}

uint64_t GetNumberOfTasksInDocument(Document* rhs) {
  return GetNumberOfTasksInSeq(rhs ? &rhs->tasks : NULL);
}

TaskNode* GetDocumentTaskAt(Document* doc, const uint64_t idx) {
  return doc ? GetTaskInSeqAt(&doc->tasks, idx) : NULL;
}

TaskNode* FindDocumentTask(Document* doc, const StrView name) {
  if (!doc)
    return NULL;

  for (size_t i = 0; i < GetNumberOfTasksInSeq(&doc->tasks); i++) {
    TaskNode* task = GetTaskInSeqAt(&doc->tasks, i);
    if (StrViewEquals(task->name, name))
      return task;
  }

  return NULL;
}

uint64_t GetNumberOfVarsInDocument(Document* rhs) {
  return GetNumberOfVarsInSeq(rhs ? &rhs->vars : NULL);
}

VarNode* GetDocumentVarAt(Document* doc, const uint64_t idx) {
  return doc ? GetVarInSeqAt(&doc->vars, idx) : NULL;
}

VarNode* FindDocumentVar(Document* doc, const StrView name) {
  if (!doc)
    return NULL;

  for (uint64_t i = 0; i < GetNumberOfVarsInSeq(&doc->vars); i++) {
    VarNode* var = GetVarInSeqAt(&doc->vars, i);
    if (StrViewEquals(var->name, name))
      return var;
  }

  return NULL;
}

VarNode* FindTaskVar(TaskNode* task, const StrView name) {
  if (!task)
    return NULL;
  for (uint64_t i = 0; i < GetNumberOfVarsInSeq(&task->vars); i++) {
    VarNode* var = GetVarInSeqAt(&task->vars, i);
    if (StrViewEquals(var->name, name))
      return var;
  }
  return NULL;
}

uint64_t GetNumberOfEnvVarsInDocument(Document* rhs) {
  return GetNumberOfVarsInSeq(rhs ? &rhs->env : NULL);
}

VarNode* GetDocumentEnvVarAt(Document* doc, const uint64_t idx) {
  return doc ? GetVarInSeqAt(&doc->env, idx) : NULL;
}

VarNode* FindDocumentEnvVar(Document* doc, const StrView name) {
  if (!doc)
    return NULL;
  for (uint64_t i = 0; i < GetNumberOfVarsInSeq(&doc->env); i++) {
    VarNode* var = GetVarInSeqAt(&doc->env, i);
    if (StrViewEquals(var->name, name))
      return var;
  }
  return NULL;
}

VarNode* FindTaskEnvVar(TaskNode* task, const StrView name) {
  if (!task)
    return NULL;
  for (uint64_t i = 0; i < GetNumberOfVarsInSeq(&task->env); i++) {
    VarNode* var = GetVarInSeqAt(&task->env, i);
    if (StrViewEquals(var->name, name))
      return var;
  }
  return NULL;
}

uint64_t GetNumberOfCommentsInDocument(Document* rhs) {
  return GetNumberOfCommentsInSeq(rhs ? &rhs->comments : NULL);
}

CommentNode* GetDocumentCommentAt(Document* doc, const uint64_t idx) {
  return doc ? GetCommentInSeqAt(&doc->comments, idx) : NULL;
}

uint64_t GetNumberOfIncludesInDocument(Document* doc) {
  return GetNumberOfIncludesInSeq(doc ? &doc->includes : NULL);
}

IncludeNode* GetDocumentIncludeAt(Document* doc, const uint64_t idx) {
  return doc ? GetIncludeInSeqAt(&doc->includes, idx) : NULL;
}

void VisitDocumentIncludes(Document* doc, IncludeVisitor vis, void* data) {
  if (!doc)
    return;
  VisitIncludesInSeq(&doc->includes, vis, data);
}

uint64_t GetNumberOfDotenvsInDocument(Document* doc) {
  return GetNumberOfStringsInSeq(doc ? &doc->dotenv : NULL);
}

StringNode* GetDocumentDotenvAt(Document* doc, uint64_t idx) {
  return doc ? GetStringInSeqAt(&doc->dotenv, idx) : NULL;
}

void VisitDocumentDotenvs(Document* doc, StringVisitor vis, void* data) {
  if (!doc)
    return;
  VisitStringsInSeq(&doc->dotenv, vis, data);
}

void VisitDocumentComments(Document* doc, CommentVisitor vis, void* data) {
  if (!doc)
    return;
  VisitCommentsInSeq(&doc->comments, vis, data);
}

void VisitDocumentCommentsMatching(Document* doc, CommentPredicate predicate, CommentVisitor vis, void* data) {
  if (!doc || !predicate || !vis)
    return;
  for (uint64_t i = 0; i < GetNumberOfCommentsInSeq(&doc->comments); i++) {
    CommentNode* value = GetCommentInSeqAt(&doc->comments, i);
    if (!predicate(value, data))
      continue;
    if (!vis(i, value, data))
      return;
  }
}

void VisitDocumentTasks(Document* doc, TaskVisitor vis, void* data) {
  if (!doc)
    return;

  VisitTasksInSeq(&doc->tasks, vis, data);
}

void VisitDocumentTasksMatching(Document* doc, TaskPredicate predicate, TaskVisitor vis, void* data) {
  if (!doc || !predicate || !vis)
    return;
  for (uint64_t i = 0; i < GetNumberOfTasksInSeq(&doc->tasks); i++) {
    TaskNode* value = GetTaskInSeqAt(&doc->tasks, i);
    if (!predicate(value, data))
      continue;
    if (!vis(i, value, data))
      return;
  }
}

void VisitDocumentVars(Document* doc, VarVisitor vis, void* data) {
  if (!doc)
    return;
  VisitVarsInSeq(&doc->vars, vis, data);
}

void VisitDocumentVarsMatching(Document* doc, VarPredicate predicate, VarVisitor vis, void* data) {
  if (!doc || !predicate || !vis)
    return;
  for (uint64_t i = 0; i < GetNumberOfVarsInSeq(&doc->vars); i++) {
    VarNode* value = GetVarInSeqAt(&doc->vars, i);
    if (!predicate(value, data))
      continue;
    if (!vis(i, value, data))
      return;
  }
}
