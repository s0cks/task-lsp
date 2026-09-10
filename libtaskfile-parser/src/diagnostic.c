#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "seq.h"
#include "taskfile_parser.h"

Diagnostic* NewDiagnosticForNode(DocumentNode* node, const DiagnosticLevel level, const Position start,
                                 const Position end, const char* fmt, ...) {
  ASSERT(node);
  Diagnostic* diagnostic = AppendNewDiagnosticInSeq(&node->diagnostics);
  if (diagnostic) {
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
  }

  return diagnostic;
}
