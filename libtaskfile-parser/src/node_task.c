#include "common.h"
#include "src/seq.h"
#include "taskfile_parser.h"

uint64_t GetNumberOfDotenvsInTask(TaskNode* rhs) {
  return GetNumberOfStringsInSeq(&rhs->dotenvs);
}

StringNode* GetTaskDotenvAt(TaskNode* node, uint64_t idx) {
  return GetStringInSeqAt(&node->dotenvs, idx);
}

void VisitTaskDotenvs(TaskNode* node, StringVisitor vis, void* data) {
  return VisitStringsInSeq(&node->dotenvs, vis, data);
}
