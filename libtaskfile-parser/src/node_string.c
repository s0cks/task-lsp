#include "common.h"
#include "seq.h"
#include "taskfile_parser.h"

uint64_t GetNumberOfRefsInString(StringNode* rhs) {
  return GetNumberOfRefsInSeq(rhs ? &rhs->refs : NULL);
}

RefNode* GetStringRefAt(StringNode* node, const uint64_t idx) {
  return node ? GetRefInSeqAt(&node->refs, idx) : NULL;
}

void VisitStringRefs(StringNode* node, RefVisitor vis, void* data) {
  if (!node)
    return;
  VisitRefsInSeq(&node->refs, vis, data);
}
