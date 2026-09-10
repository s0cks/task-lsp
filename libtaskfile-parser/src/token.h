#ifndef TASKFILE_PARSER_TOKEN_H
#define TASKFILE_PARSER_TOKEN_H

#include "taskfile_parser.h"

#ifdef __cplusplus
#include <iostream>
extern "C" {
#endif  // __cplusplus

#define FOR_EACH_TOKEN_KIND(V) \
  V(Key)                       \
  V(Colon)                     \
  V(String)                    \
  V(Number)                    \
  V(BlockComment)              \
  V(LineComment)               \
  V(DocumentSeparator)

// clang-format off
typedef enum {
  kInvalidToken = 0,
#define DEFINE_KIND(Name) k##Name##Token,
  FOR_EACH_TOKEN_KIND(DEFINE_KIND)
#undef DEFINE_KIND
  kArrowToken,
  kPipeToken,
  kHashToken,
  kDashToken,
  kEofToken,
  kTotalNumberOfTokenKinds,
} TokenKind;
// clang-format on

typedef struct {
  TokenKind kind;
  StrView data;
  Range range;
} Token;

#ifdef __cplusplus
};

static inline auto operator<<(std::ostream& stream, const TokenKind& rhs) -> std::ostream& {
  switch (rhs) {
    case kInvalidToken:
      return stream << "Invalid";
    default:
      return stream << "Unknown";
  }
}

static inline auto operator<<(std::ostream& stream, const Token& rhs) -> std::ostream& {
  stream << "Token{";
  stream << "kind=" << rhs.kind << ", ";
  stream << "data=" << std::string(rhs.data.start, rhs.data.len) << ", ";
  stream << "range=" << rhs.range;
  return stream;
}
#endif  // __cplusplus

#endif  // TASKFILE_PARSER_TOKEN_H
