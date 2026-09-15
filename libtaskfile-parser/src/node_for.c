#include "common.h"
#include "seq.h"
#include "taskfile_parser.h"

uint64_t GetNumberOfItemsInFor(ForNode* rhs) {
  return GetNumberOfStringsInSeq(rhs ? &rhs->items : NULL);
}

StringNode* GetForItemAt(ForNode* node, const uint64_t idx) {
  return node ? GetStringInSeqAt(&node->items, idx) : NULL;
}

void VisitForItems(ForNode* node, StringVisitor vis, void* data) {
  if (!node)
    return;
  VisitStringsInSeq(&node->items, vis, data);
}
