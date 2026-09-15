#include "common.h"
#include "document.h"
#include "seq.h"
#include "taskfile_parser.h"

static inline bool PosAtOrAfter(const Pos a, const Pos b) {
  return a.row > b.row || (a.row == b.row && a.col >= b.col);
}

static inline bool PosAtOrBefore(const Pos a, const Pos b) {
  return a.row < b.row || (a.row == b.row && a.col <= b.col);
}

static inline bool RangeContainsPos(const Range range, const Pos pos) {
  return PosAtOrAfter(pos, range.start) && PosAtOrBefore(pos, range.end);
}

static VisitResult VisitChild(DocumentNode* child, DocumentNodeVisitor before, DocumentNodeVisitor after, void* data) {
  if (!child)
    return kVisitContinue;
  VisitNode(child, before, after, data);
  return kVisitContinue;
}

static void VisitVarChildren(VarNode* var, DocumentNodeVisitor before, DocumentNodeVisitor after, void* data) {
  switch (var->var_kind) {
    case kScalarVarNodeKind:
      VisitChild(var->value, before, after, data);
      break;
    case kShellVarNodeKind:
      VisitChild((DocumentNode*)var->command, before, after, data);
      break;
    case kRefVarNodeKind:
      VisitChild((DocumentNode*)var->ref, before, after, data);
      break;
    case kMapVarNodeKind:
      VisitChild(var->value, before, after, data);
      break;
    default:
      break;
  }
}

void VisitNode(DocumentNode* node, DocumentNodeVisitor before, DocumentNodeVisitor after, void* data) {
  if (!node)
    return;

  if (before && before(node, data) == kVisitStop)
    return;

  switch (node->kind) {
    case kTaskKind: {
      TaskNode* task = (TaskNode*)node;
      for (uint64_t i = 0; i < GetNumberOfCommandsInSeq(&task->cmds); i++)
        VisitChild((DocumentNode*)GetCommandInSeqAt(&task->cmds, i), before, after, data);
      for (uint64_t i = 0; i < GetNumberOfRefsInSeq(&task->deps); i++)
        VisitChild((DocumentNode*)GetRefInSeqAt(&task->deps, i), before, after, data);
      for (uint64_t i = 0; i < GetNumberOfVarsInSeq(&task->vars); i++)
        VisitChild((DocumentNode*)GetVarInSeqAt(&task->vars, i), before, after, data);
      for (uint64_t i = 0; i < GetNumberOfVarsInSeq(&task->env); i++)
        VisitChild((DocumentNode*)GetVarInSeqAt(&task->env, i), before, after, data);
      for (uint64_t i = 0; i < GetNumberOfCommandsInSeq(&task->status_cmds); i++)
        VisitChild((DocumentNode*)GetCommandInSeqAt(&task->status_cmds, i), before, after, data);
      for (uint64_t i = 0; i < GetNumberOfPreconditionsInSeq(&task->preconditions); i++)
        VisitChild((DocumentNode*)GetPreconditionInSeqAt(&task->preconditions, i), before, after, data);
#define VISIT_TASK_STRING_SEQ(Singular, Plural, Field)                 \
  for (uint64_t i = 0; i < GetNumberOfStringsInSeq(&task->Field); i++) \
    VisitChild((DocumentNode*)GetStringInSeqAt(&task->Field, i), before, after, data);
      FOR_EACH_TASK_STRING_SEQ(VISIT_TASK_STRING_SEQ)
#undef VISIT_TASK_STRING_SEQ
      for (uint64_t i = 0; i < GetNumberOfStringsInSeq(&task->dotenvs); i++)
        VisitChild((DocumentNode*)GetStringInSeqAt(&task->dotenvs, i), before, after, data);
      break;
    }

    case kCommandKind: {
      CommandNode* cmd = (CommandNode*)node;
      if (IsCommandTaskCall(cmd))
        VisitChild((DocumentNode*)&cmd->task_call, before, after, data);
      else
        VisitChild((DocumentNode*)&cmd->cmd, before, after, data);
      VisitChild((DocumentNode*)cmd->if_expr, before, after, data);
      VisitChild((DocumentNode*)cmd->for_each, before, after, data);
      VisitChild((DocumentNode*)cmd->defer, before, after, data);
      for (uint64_t i = 0; i < GetNumberOfStringsInSeq(&cmd->platforms); i++)
        VisitChild((DocumentNode*)GetStringInSeqAt(&cmd->platforms, i), before, after, data);
      break;
    }

    case kForKind: {
      ForNode* loop = (ForNode*)node;
      for (uint64_t i = 0; i < GetNumberOfItemsInFor(loop); i++)
        VisitChild((DocumentNode*)GetForItemAt(loop, i), before, after, data);
      if (loop->for_kind == kVarForKind)
        VisitChild((DocumentNode*)&loop->var, before, after, data);
      VisitChild((DocumentNode*)loop->matrix, before, after, data);
      break;
    }

    case kDeferKind: {
      DeferNode* defer = (DeferNode*)node;
      if (IsDeferTaskCall(defer))
        VisitChild((DocumentNode*)&defer->task_call, before, after, data);
      VisitChild((DocumentNode*)defer->cmd, before, after, data);
      break;
    }

    case kIncludeKind: {
      IncludeNode* include = (IncludeNode*)node;
      for (uint64_t i = 0; i < GetNumberOfIncludeAliases(include); i++)
        VisitChild((DocumentNode*)GetIncludeAliasAt(include, i), before, after, data);
      for (uint64_t i = 0; i < GetNumberOfIncludeExcludes(include); i++)
        VisitChild((DocumentNode*)GetIncludeExcludeAt(include, i), before, after, data);
      for (uint64_t i = 0; i < GetNumberOfVarsInSeq(&include->vars); i++)
        VisitChild((DocumentNode*)GetVarInSeqAt(&include->vars, i), before, after, data);
      break;
    }

    case kStringKind: {
      StringNode* str = (StringNode*)node;
      for (uint64_t i = 0; i < GetNumberOfRefsInString(str); i++)
        VisitChild((DocumentNode*)GetStringRefAt(str, i), before, after, data);
      break;
    }

    case kRefKind: {
      RefNode* ref = (RefNode*)node;
      for (uint64_t i = 0; i < GetNumberOfPipelineExprsInSeq(&ref->pipeline); i++)
        VisitChild((DocumentNode*)GetPipelineExprInSeqAt(&ref->pipeline, i), before, after, data);
      break;
    }

    case kVarKind:
      VisitVarChildren((VarNode*)node, before, after, data);
      break;

    case kMapKind: {
      MapNode* map = (MapNode*)node;
      for (uint64_t i = 0; i < GetNumberOfMapEntrysInSeq(&map->entries); i++)
        VisitChild((DocumentNode*)GetMapEntryInSeqAt(&map->entries, i), before, after, data);
      break;
    }

    case kMapEntryKind:
      VisitChild(((MapEntryNode*)node)->value, before, after, data);
      break;

    case kPreconditionKind: {
      PreconditionNode* pre = (PreconditionNode*)node;
      VisitChild((DocumentNode*)pre->command, before, after, data);
      VisitChild((DocumentNode*)pre->message, before, after, data);
      break;
    }

    default:
      break;
  }

  if (after)
    after(node, data);
}

void VisitDocument(Document* doc, DocumentNodeVisitor before, DocumentNodeVisitor after, void* data) {
  if (!doc)
    return;

  for (uint64_t i = 0; i < GetNumberOfVarsInDocument(doc); i++)
    VisitNode((DocumentNode*)GetDocumentVarAt(doc, i), before, after, data);
  for (uint64_t i = 0; i < GetNumberOfEnvVarsInDocument(doc); i++)
    VisitNode((DocumentNode*)GetDocumentEnvVarAt(doc, i), before, after, data);
  for (uint64_t i = 0; i < GetNumberOfIncludesInDocument(doc); i++)
    VisitNode((DocumentNode*)GetDocumentIncludeAt(doc, i), before, after, data);
  for (uint64_t i = 0; i < GetNumberOfDotenvsInDocument(doc); i++)
    VisitNode((DocumentNode*)GetDocumentDotenvAt(doc, i), before, after, data);
  for (uint64_t i = 0; i < GetNumberOfTasksInDocument(doc); i++)
    VisitNode((DocumentNode*)GetDocumentTaskAt(doc, i), before, after, data);
}

typedef struct {
  Pos pos;
  DocumentNode* best;
} FindAtPositionCtx;

static VisitResult NarrowToPosition(DocumentNode* node, void* raw) {
  FindAtPositionCtx* ctx = (FindAtPositionCtx*)raw;
  if (!RangeContainsPos((Range){node->start, node->end}, ctx->pos))
    return kVisitStop;
  ctx->best = node;
  return kVisitContinue;
}

DocumentNode* FindNodeAtPosition(Document* doc, const Pos pos) {
  FindAtPositionCtx ctx = {.pos = pos, .best = NULL};
  VisitDocument(doc, NarrowToPosition, NULL, &ctx);
  return ctx.best;
}
