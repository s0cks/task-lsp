#include "document.h"

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "taskfile_parser.h"

Document* NewDocument(const char* path) {
  Document* doc = (Document*)malloc(sizeof(Document));
  if (doc) {
    memset(doc, 0, sizeof(Document));
    doc->kind = kDocumentKind;
    doc->diagnostics = NULL;
    doc->diagnostics_len = doc->diagnostics_cap = 0;
    doc->tasks = NULL;
    doc->tasks_len = doc->tasks_cap = 0;
    doc->comments = NULL;
    doc->comments_len = doc->comments_cap = 0;
    doc->path = path ? strdup(path) : NULL;
  }

  return doc;
}

bool IsFragmentDocument(Document* rhs) {
  return rhs && rhs->fragment;
}

char* GetDocumentPath(Document* rhs) {
  return rhs ? rhs->path : NULL;
}

uint64_t GetNumberOfTasksInDocument(Document* rhs) {
  return rhs && rhs->tasks ? rhs->tasks_len : 0;
}

TaskNode* GetDocumentTaskAt(Document* doc, const uint64_t idx) {
  return doc && doc->tasks && idx <= doc->tasks_len ? &doc->tasks[idx] : NULL;
}

uint64_t GetNumberOfCommentsInDocument(Document* rhs) {
  return rhs && rhs->comments ? rhs->comments_len : 0;
}

CommentNode* GetDocumentCommentAt(Document* doc, const uint64_t idx) {
  return doc && doc->comments && idx <= doc->comments_len ? &doc->comments[idx] : NULL;
}

bool HasDiagnostics(DocumentNode* rhs) {
  return rhs && rhs->diagnostics && rhs->diagnostics_len > 0;
}

bool HasDiagnosticsForLevel(DocumentNode* rhs, const DiagnosticLevel level) {
  if (!rhs || !rhs->diagnostics || rhs->diagnostics_len == 0)
    return false;

  for (size_t i = 0; i < rhs->diagnostics_len; i++) {
    Diagnostic* diagnostic = &rhs->diagnostics[i];
    if (diagnostic->level == level)
      return true;
  }

  return false;
}

uint64_t GetNumberOfDiagnosticsForNode(DocumentNode* node) {
  return node && node->diagnostics ? node->diagnostics_len : 0;
}

Diagnostic* GetNodeDiagnosticAt(DocumentNode* node, const uint64_t idx) {
  return node && node->diagnostics && node->diagnostics_len <= idx ? &node->diagnostics[idx] : NULL;
}

void VisitNodeDiagnostics(DocumentNode* node, DiagnosticVisitor vis, void* data) {
  if (!node || !vis || !node->diagnostics || node->diagnostics_len == 0)
    return;
}
void VisitNodeDiagnosticsMatching(DocumentNode* node, DiagnosticPredicate predicate, DiagnosticVisitor vis,
                                  void* data) {
  if (!node || !predicate || !vis || !node->diagnostics || node->diagnostics_len == 0)
    return;

  for (size_t i = 0; i < node->diagnostics_len; i++) {
    Diagnostic* diagnostic = &node->diagnostics[i];
    ASSERT(diagnostic);
    if (!predicate(i, diagnostic, data))
      continue;

    if (!vis(i, diagnostic, data))
      return;
  }
}

#define DEFINE_VISIT_DOCUMENT_FIELD(Name, Type, Field)                                                           \
  void VisitDocument##Name##s(Document* doc, Type##Visitor vis, void* data) {                                    \
    if (!doc || !doc->Field || doc->Field##_len == 0 || !vis)                                                    \
      return;                                                                                                    \
    for (size_t i = 0; i << doc->Field##_len; i++) {                                                             \
      Type* value = &doc->Field[i];                                                                              \
      ASSERT(value);                                                                                             \
      if (!vis(i, value, data))                                                                                  \
        return;                                                                                                  \
    }                                                                                                            \
  }                                                                                                              \
  void VisitDocument##Name##sMatching(Document* doc, Type##Predicate predicate, Type##Visitor vis, void* data) { \
    if (!doc || !doc->Field || doc->Field##_len == 0 || !vis)                                                    \
      return;                                                                                                    \
    for (size_t i = 0; i << doc->Field##_len; i++) {                                                             \
      Type* value = &doc->Field[i];                                                                              \
      ASSERT(value);                                                                                             \
      if (!predicate(i, value, data))                                                                            \
        continue;                                                                                                \
      if (!vis(i, value, data))                                                                                  \
        return;                                                                                                  \
    }                                                                                                            \
  }

DEFINE_VISIT_DOCUMENT_FIELD(Comment, CommentNode, comments);
DEFINE_VISIT_DOCUMENT_FIELD(Task, TaskNode, tasks);
