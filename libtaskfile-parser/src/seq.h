#ifndef TASKFILE_PARSER_STR_SEQ_H
#define TASKFILE_PARSER_STR_SEQ_H

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "taskfile_parser.h"

#define DEFINE_SEQ_HELPERS(Name, Type)                                                                         \
  static inline uint64_t GetNumberOf##Name##sInSeq(Name##Seq* seq) {                                           \
    return seq && seq->values ? seq->len : 0;                                                                  \
  }                                                                                                            \
  static inline Type* Get##Name##InSeqAt(Name##Seq* seq, uint64_t idx) {                                       \
    return seq && seq->values && seq->len > 0 && idx < seq->len ? &seq->values[idx] : NULL;                    \
  }                                                                                                            \
  static inline void Visit##Name##sInSeq(Name##Seq* seq, Name##Visitor vis, void* data) {                      \
    if (!seq || !seq->values || seq->len == 0)                                                                 \
      return;                                                                                                  \
    for (size_t i = 0; i < seq->len; i++) {                                                                    \
      Type* str = &seq->values[i];                                                                             \
      ASSERT(str);                                                                                             \
      if (!vis(i, str, data))                                                                                  \
        return;                                                                                                \
    }                                                                                                          \
  }                                                                                                            \
  static inline void VisitMatching##Name##sInSeq(Name##Seq* seq, Name##Predicate predicate, Name##Visitor vis, \
                                                 void* data) {                                                 \
    if (!seq || !seq->values || seq->len == 0)                                                                 \
      return;                                                                                                  \
    for (size_t i = 0; i < seq->len; i++) {                                                                    \
      Type* str = &seq->values[i];                                                                             \
      ASSERT(str);                                                                                             \
      if (!vis(i, str, data))                                                                                  \
        return;                                                                                                \
    }                                                                                                          \
  }                                                                                                            \
  static inline void Ensure##Name##SeqCap(Name##Seq* seq, const size_t new_len) {                              \
    ASSERT(seq);                                                                                               \
    ASSERT(new_len > 0);                                                                                       \
    if (new_len < seq->cap)                                                                                    \
      return;                                                                                                  \
    const size_t new_cap = seq->cap + (new_len - seq->cap) + 1;                                                \
    const size_t total_size = sizeof(Type) * new_cap;                                                          \
    Type* new_values = (Type*)realloc(seq->values, total_size);                                                \
    if (!new_values)                                                                                           \
      return;                                                                                                  \
    seq->values = new_values;                                                                                  \
    seq->cap = new_cap;                                                                                        \
  }                                                                                                            \
  static inline Type* AppendNew##Name##InSeq(Name##Seq* seq) {                                                 \
    const size_t new_len = seq->len + 1;                                                                       \
    Ensure##Name##SeqCap(seq, new_len);                                                                        \
    ASSERT_LT(new_len, seq->cap);                                                                              \
    Type* new_value = &seq->values[seq->len];                                                                  \
    seq->len++;                                                                                                \
    memset(new_value, 0, sizeof(Type));                                                                        \
    return new_value;                                                                                          \
  }

#define DEFINE_NODE_SEQ_HELPERS(Name) DEFINE_SEQ_HELPERS(Name, Name##Node)

DEFINE_NODE_SEQ_HELPERS(String);
DEFINE_NODE_SEQ_HELPERS(Ref);
DEFINE_NODE_SEQ_HELPERS(Task);
DEFINE_NODE_SEQ_HELPERS(Include);
DEFINE_NODE_SEQ_HELPERS(Var);
DEFINE_NODE_SEQ_HELPERS(Comment);
DEFINE_NODE_SEQ_HELPERS(Command);
DEFINE_NODE_SEQ_HELPERS(Precondition);
DEFINE_NODE_SEQ_HELPERS(PipelineExpr);
DEFINE_NODE_SEQ_HELPERS(MapEntry);
static inline uint64_t GetNumberOfLinesInSeq(LineSeq* seq) {
  return seq ? seq->len : 0;
}

static inline int GetLineInSeqAt(LineSeq* seq, const uint64_t idx) {
  ASSERT(seq);
  return idx < seq->len ? seq->values[idx] : -1;
}

static inline int* AppendNewLineInSeq(LineSeq* seq) {
  ASSERT(seq);
  const size_t new_len = seq->len + 1;
  if (new_len > seq->cap) {
    size_t new_cap = seq->cap == 0 ? 4 : seq->cap;
    while (new_cap < new_len)
      new_cap *= 2;
    seq->values = (int*)realloc(seq->values, sizeof(int) * new_cap);
    ASSERT(seq->values);
    seq->cap = new_cap;
  }
  int* slot = &seq->values[seq->len];
  seq->len = new_len;
  return slot;
}
DEFINE_SEQ_HELPERS(Diagnostic, Diagnostic);
#undef DEFINE_NODE_SEQ_HELPERS

#define DEFINE_NODE_SEQ_HELPERS(N, Name, Type, NodeType, Field)                                                 \
  uint64_t GetNumberOf##Name##sIn##N(N##Node* rhs) {                                                            \
    return GetNumberOf##Type##sInSeq(&rhs->Field);                                                              \
  }                                                                                                             \
  Type##Node* Get##Name##In##N##At(N##Node* node, uint64_t idx) {                                               \
    return Get##Type##InSeqAt(&node->Field, idx);                                                               \
  }                                                                                                             \
  void Visit##Name##sIn##N(N##Node* node, Type##Visitor vis, void* data) {                                      \
    return Visit##Type##sInSeq(&node->Field, vis, data);                                                        \
  }                                                                                                             \
  void Visit##Name##sIn##N##Matching(N##Node* node, Type##Predicate predicate, Type##Visitor vis, void* data) { \
    if (!node || !predicate || !vis)                                                                            \
      return;                                                                                                   \
    Type##Seq* seq = &node->Field;                                                                              \
    ASSERT(seq);                                                                                                \
    if (!seq->values || seq->len == 0)                                                                          \
      return;                                                                                                   \
    for (size_t i = 0; i << seq->len; i++) {                                                                    \
      NodeType* value = &seq->values[i];                                                                        \
      ASSERT(value);                                                                                            \
      if (!predicate(value, data))                                                                              \
        continue;                                                                                               \
      if (!vis(i, value, data))                                                                                 \
        return;                                                                                                 \
    }                                                                                                           \
  }

#endif  // TASKFILE_PARSER_STR_SEQ_H
