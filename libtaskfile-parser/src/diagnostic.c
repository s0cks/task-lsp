#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "taskfile_parser.h"

static inline void EnsureCap(DocumentNode* node, const size_t new_len) {
  ASSERT(node);
  ASSERT(new_len > 0);
  if (new_len < node->diagnostics_cap)
    return;

  const size_t new_cap = node->diagnostics_cap + (new_len - node->diagnostics_cap) + 1;
  const size_t total_size = sizeof(Diagnostic) * new_cap;
  Diagnostic* new_diagnostics = (Diagnostic*)realloc(node->diagnostics, total_size);
  if (!new_diagnostics)
    return;

  node->diagnostics = new_diagnostics;
  node->diagnostics_cap = new_cap;
}

Diagnostic* NewDiagnosticForNode(DocumentNode* node, const DiagnosticLevel level, const Position start,
                                 const Position end, const char* fmt, ...) {
  ASSERT(node);
  EnsureCap(node, node->diagnostics_len + 1);
  ASSERT_LT(node->diagnostics_len + 1, node->diagnostics_cap);
  Diagnostic* diagnostic = &node->diagnostics[node->diagnostics_len];
  node->diagnostics_len++;
  memset(diagnostic, 0, sizeof(Diagnostic));
  diagnostic->owner = node;
  diagnostic->level = level;
  memmove(&diagnostic->start, &start, sizeof(Position));
  memmove(&diagnostic->end, &end, sizeof(Position));

  char* message = NULL;
  if (fmt != NULL) {
    va_list args;
    va_start(args, fmt);
    const size_t message_len = snprintf(NULL, 0, fmt, args);
    const size_t total_message_len = message_len + 1;
    message = (char*)calloc(sizeof(char), total_message_len);
    if (message)
      snprintf(message, total_message_len, fmt, args);
    va_end(args);
  }
  diagnostic->message = message;

  return diagnostic;
}
