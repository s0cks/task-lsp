#include "common.h"
#include "taskfile_parser.h"

uint64_t GetNumberOfRefsInString(StringNode* rhs) {
  return rhs && rhs->refs ? rhs->refs_len : 0;
}

RefNode* GetStringRefAt(StringNode* node, uint64_t idx) {
  return node && node->refs && idx < node->refs_len ? &node->refs[idx] : NULL;
}

void VisitStringRefs(StringNode* node, RefVisitor vis, void* data) {
  if (!node || !node->refs || node->refs_len == 0)
    return;

  for (size_t i = 0; i < node->refs_len; i++) {
    RefNode* ref = &node->refs[i];
    ASSERT(ref);
    if (!vis(i, ref, data))
      return;
  }
}
