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
