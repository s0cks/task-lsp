#include "common.h"
#include "seq.h"
#include "taskfile_parser.h"

uint64_t GetNumberOfDotenvsInTask(TaskNode* rhs) {
  return GetNumberOfStringsInSeq(rhs ? &rhs->dotenvs : NULL);
}

StringNode* GetTaskDotenvAt(TaskNode* node, const uint64_t idx) {
  return node ? GetStringInSeqAt(&node->dotenvs, idx) : NULL;
}

void VisitTaskDotenvs(TaskNode* node, StringVisitor vis, void* data) {
  if (!node)
    return;
  VisitStringsInSeq(&node->dotenvs, vis, data);
}

void VisitTaskDeps(TaskNode* node, RefVisitor vis, void* data) {
  if (!node)
    return;

  return VisitRefsInSeq(&node->deps, vis, data);
}

#define DEFINE_TASK_STRING_SEQ(Singular, Plural, Field)                   \
  uint64_t GetNumberOf##Plural##InTask(TaskNode* rhs) {                   \
    return GetNumberOfStringsInSeq(rhs ? &rhs->Field : NULL);             \
  }                                                                       \
  StringNode* GetTask##Singular##At(TaskNode* node, const uint64_t idx) { \
    return node ? GetStringInSeqAt(&node->Field, idx) : NULL;             \
  }                                                                       \
  void VisitTask##Plural(TaskNode* node, StringVisitor vis, void* data) { \
    if (!node)                                                            \
      return;                                                             \
    VisitStringsInSeq(&node->Field, vis, data);                           \
  }

FOR_EACH_TASK_STRING_SEQ(DEFINE_TASK_STRING_SEQ)
#undef DEFINE_TASK_STRING_SEQ
