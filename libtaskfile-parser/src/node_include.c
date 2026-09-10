#include "common.h"
#include "src/seq.h"
#include "taskfile_parser.h"

uint64_t GetNumberOfIncludeAliases(IncludeNode* rhs) {
  return GetNumberOfStringsInSeq(&rhs->aliases);
}

StringNode* GetIncludeAliasAt(IncludeNode* node, uint64_t idx) {
  return GetStringInSeqAt(&node->aliases, idx);
}

void VisitIncludeAliases(IncludeNode* node, StringVisitor vis, void* data) {
  return VisitStringsInSeq(&node->aliases, vis, data);
}

uint64_t GetNumberOfIncludeExcludes(IncludeNode* rhs) {
  return GetNumberOfStringsInSeq(&rhs->excludes);
}

StringNode* GetIncludeExcludeAt(IncludeNode* node, uint64_t idx) {
  return GetStringInSeqAt(&node->excludes, idx);
}

void VisitIncludeExcludes(IncludeNode* node, StringVisitor vis, void* data) {
  return VisitStringsInSeq(&node->excludes, vis, data);
}
