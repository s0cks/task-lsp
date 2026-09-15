#ifndef TASKFILE_PARSER_TOKEN_ASSERTIONS_H
#define TASKFILE_PARSER_TOKEN_ASSERTIONS_H

#include "gtest/gtest.h"
#ifdef __cplusplus

#include <gtest/gtest.h>

#include "lexer.h"
#include "token.h"

static inline auto IsTokenKind(const Token& lhs, const TokenKind rhs) -> ::testing::AssertionResult {
  if (lhs.kind != rhs)
    return ::testing::AssertionFailure() << "expected " << lhs << " to be a " << rhs << ", but was a " << lhs.kind;
  return ::testing::AssertionSuccess() << lhs << " is a " << rhs;
}

static inline auto IsTokenKindNext(Lexer* lex, const TokenKind rhs) -> ::testing::AssertionResult {
  const Token next = LexerNext(lex);
  return IsTokenKind(next, rhs);
}

static inline auto HasTokenText(const Token& lhs, const std::string& rhs) -> ::testing::AssertionResult {
  const std::string data(lhs.data);
  if (data != rhs)
    return ::testing::AssertionFailure() << "expected " << lhs << " to have text `" << rhs << "`, but was `" << data
                                         << "`";
  return ::testing::AssertionSuccess();
}

#define DEFINE_TOKEN_ASSERTIONS(Name)                                                                          \
  static inline auto Is##Name##Token(const Token& rhs) -> ::testing::AssertionResult {                         \
    return IsTokenKind(rhs, k##Name##Token);                                                                   \
  }                                                                                                            \
  static inline auto Is##Name##TokenNext(Lexer* lex) -> ::testing::AssertionResult {                           \
    return IsTokenKindNext(lex, k##Name##Token);                                                               \
  }                                                                                                            \
  static inline auto Is##Name##Token(const Token& lhs, const std::string& rhs) -> ::testing::AssertionResult { \
    const auto kind_result = IsTokenKind(lhs, k##Name##Token);                                                 \
    if (!kind_result)                                                                                          \
      return kind_result;                                                                                      \
    return HasTokenText(lhs, rhs);                                                                             \
  }                                                                                                            \
  static inline auto Is##Name##TokenNext(Lexer* lex, const std::string& rhs) -> ::testing::AssertionResult {   \
    const Token next = LexerNext(lex);                                                                         \
    return Is##Name##Token(next, rhs);                                                                         \
  }

DEFINE_TOKEN_ASSERTIONS(Eof);
DEFINE_TOKEN_ASSERTIONS(Invalid);
DEFINE_TOKEN_ASSERTIONS(Dash);
DEFINE_TOKEN_ASSERTIONS(DocumentSeparator);
DEFINE_TOKEN_ASSERTIONS(Key);
DEFINE_TOKEN_ASSERTIONS(String);
DEFINE_TOKEN_ASSERTIONS(Number);
DEFINE_TOKEN_ASSERTIONS(BlockScalarHeader);
DEFINE_TOKEN_ASSERTIONS(LineComment);
DEFINE_TOKEN_ASSERTIONS(BlockComment);

#undef DEFINE_TOKEN_ASSERTIONS

#endif  // __cplusplus

#endif  // TASKFILE_PARSER_TOKEN_ASSERTIONS_H
