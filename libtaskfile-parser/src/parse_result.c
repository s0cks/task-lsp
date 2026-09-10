#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "taskfile_parser.h"

char* TaskfileParseResultToStr(TaskfileParseResult* rhs) {
  static const char* kSuccessMessage = "success";
  static const char* kErrorFormat = "error: %s";
  if (TaskfileParseResultIsOk(rhs))
    return strdup(kSuccessMessage);

  const size_t len = snprintf(NULL, 0, kErrorFormat, rhs->msg);
  if (len < 0)
    return NULL;
  const size_t total_size = len + 1;

  char* message = (char*)calloc(sizeof(char), total_size);
  if (!message)
    return NULL;

  snprintf(message, total_size, kErrorFormat, rhs->msg);
  return message;
}

void FreeTaskfileParseResult(TaskfileParseResult* rhs) {
  if (!rhs)
    return;

  if (!rhs->success && rhs->msg)
    free(rhs->msg);
}
