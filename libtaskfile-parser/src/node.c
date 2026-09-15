#include <stdio.h>

#include "seq.h"
#include "taskfile_parser.h"

bool HasDiagnosticsForLevel(DocumentNode* rhs, const DiagnosticLevel level) {
  fprintf(stderr, "%s not implemented", __PRETTY_FUNCTION__);
  return false;
}

uint64_t GetNumberOfDiagnosticsForNode(DocumentNode* node) {
  return node ? GetNumberOfDiagnosticsInSeq(&node->diagnostics) : 0;
}

Diagnostic* GetNodeDiagnosticAt(DocumentNode* node, const uint64_t idx) {
  return node ? GetDiagnosticInSeqAt(&node->diagnostics, idx) : NULL;
}

void VisitNodeDiagnostics(DocumentNode* node, DiagnosticVisitor vis, void* data) {
  if (!node)
    return;
  return VisitDiagnosticsInSeq(&node->diagnostics, vis, data);
}

void VisitNodeDiagnosticsMatching(DocumentNode* node, DiagnosticPredicate predicate, DiagnosticVisitor vis,
                                  void* data) {
  if (!node || !predicate || !vis)
    return;
  return VisitMatchingDiagnosticsInSeq(&node->diagnostics, predicate, vis, data);
}

uint64_t GetNumberOfCommentsForNode(DocumentNode* node) {
  return node ? GetNumberOfCommentsInSeq(&node->comments) : 0;
}

CommentNode* GetNodeCommentAt(DocumentNode* node, const uint64_t idx) {
  return node ? GetCommentInSeqAt(&node->comments, idx) : NULL;
}

void VisitNodeComments(DocumentNode* node, CommentVisitor vis, void* data) {
  if (!node)
    return;

  for (size_t i = 0; i < node->comments.len; i++) {
    CommentNode* n = &node->comments.values[i];
    if (!vis(i, n, data))
      return;
  }
}

void VisitNodeCommentsMatching(DocumentNode* node, CommentPredicate predicate, CommentVisitor vis, void* data) {
  if (!node)
    return;

  for (size_t i = 0; i < node->comments.len; i++) {
    CommentNode* n = &node->comments.values[i];
    if (!predicate(n, data))
      continue;

    if (!vis(i, n, data))
      return;
  }
}
