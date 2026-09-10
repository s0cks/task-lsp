#include "common.h"
#include "taskfile_parser.h"

uint64_t GetNumberOfDotenvsInTask(TaskNode* rhs) {
  return rhs && rhs->dotenv ? rhs->dotenv_len : 0;
}

StringNode* GetTaskDotenvAt(TaskNode* node, uint64_t idx) {
  return node && node->dotenv && idx < node->dotenv_len ? &node->dotenv[idx] : NULL;
}

void VisitTaskDotenvs(TaskNode* node, StringVisitor vis, void* data) {
  if (!node || !node->dotenv || node->dotenv_len == 0)
    return;

  for (size_t i = 0; i < node->dotenv_len; i++) {
    StringNode* dotenv = &node->dotenv[i];
    ASSERT(dotenv);
    if (!vis(i, dotenv, data))
      return;
  }
}
