#include "common.h"
#include "seq.h"
#include "taskfile_parser.h"

uint64_t GetNumberOfIncludeAliases(IncludeNode* rhs) {
  return GetNumberOfStringsInSeq(rhs ? &rhs->aliases : NULL);
}

StringNode* GetIncludeAliasAt(IncludeNode* node, const uint64_t idx) {
  return node ? GetStringInSeqAt(&node->aliases, idx) : NULL;
}

void VisitIncludeAliases(IncludeNode* node, StringVisitor vis, void* data) {
  if (!node)
    return;
  VisitStringsInSeq(&node->aliases, vis, data);
}

uint64_t GetNumberOfIncludeExcludes(IncludeNode* rhs) {
  return GetNumberOfStringsInSeq(rhs ? &rhs->excludes : NULL);
}

StringNode* GetIncludeExcludeAt(IncludeNode* node, const uint64_t idx) {
  return node ? GetStringInSeqAt(&node->excludes, idx) : NULL;
}

void VisitIncludeExcludes(IncludeNode* node, StringVisitor vis, void* data) {
  if (!node)
    return;
  VisitStringsInSeq(&node->excludes, vis, data);
}
