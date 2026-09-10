#include "common.h"
#include "taskfile_parser.h"

uint64_t GetNumberOfIncludeAliases(IncludeNode* rhs) {
  return rhs && rhs->aliases ? rhs->aliases_len : 0;
}

StringNode* GetIncludeAliasAt(IncludeNode* node, uint64_t idx) {
  return node && node->aliases && idx < node->aliases_len ? &node->aliases[idx] : NULL;
}

void VisitIncludeAliases(IncludeNode* node, StringVisitor vis, void* data) {
  if (!node || !node->aliases || node->aliases_len == 0)
    return;

  for (size_t i = 0; i < node->aliases_len; i++) {
    StringNode* alias = &node->aliases[i];
    ASSERT(alias);
    if (!vis(i, alias, data))
      return;
  }
}

uint64_t GetNumberOfIncludeExcludes(IncludeNode* rhs) {
  return rhs && rhs->excludes ? rhs->excludes_len : 0;
}

StringNode* GetIncludeExcludeAt(IncludeNode* node, uint64_t idx) {
  return node && node->excludes && idx < node->excludes_len ? &node->excludes[idx] : NULL;
}

void VisitIncludeExcludes(IncludeNode* node, StringVisitor vis, void* data) {
  if (!node || !node->excludes || node->excludes_len == 0)
    return;

  for (size_t i = 0; i < node->excludes_len; i++) {
    StringNode* exclude = &node->excludes[i];
    ASSERT(exclude);
    if (!vis(i, exclude, data))
      return;
  }
}
