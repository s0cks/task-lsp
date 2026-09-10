#ifndef TASKFILE_PARSER_TOKEN_ASSERTIONS_H
#define TASKFILE_PARSER_TOKEN_ASSERTIONS_H

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

#define DEFINE_TOKEN_ASSERTIONS(Name)                                                  \
  static inline auto Is##Name##Token(const Token& rhs) -> ::testing::AssertionResult { \
    return IsTokenKind(rhs, k##Name##Token);                                           \
  }                                                                                    \
  static inline auto Is##Name##TokenNext(Lexer* lex) -> ::testing::AssertionResult {   \
    return IsTokenKindNext(lex, k##Name##Token);                                       \
  }

DEFINE_TOKEN_ASSERTIONS(Eof);
DEFINE_TOKEN_ASSERTIONS(Invalid);
DEFINE_TOKEN_ASSERTIONS(Dash);
DEFINE_TOKEN_ASSERTIONS(DocumentSeparator);
DEFINE_TOKEN_ASSERTIONS(Vars);
DEFINE_TOKEN_ASSERTIONS(Tasks);

DEFINE_TOKEN_ASSERTIONS(LineComment);

static inline auto IsLineCommentToken(const Token& lhs, const std::string message) -> testing::AssertionResult {
  if (lhs.kind != kLineCommentToken)
    return ::testing::AssertionFailure() << "expected token to be a " << kLineCommentToken << ", but was: " << lhs;

  if (((std::string)lhs.data) != message)
    return ::testing::AssertionFailure() << "expected " << kLineCommentToken << " to have the message `" << message
                                         << "` but was: " << lhs.data;

  return ::testing::AssertionSuccess();
}

static inline auto IsLineCommentTokenNext(Lexer* lex, const std::string message) -> testing::AssertionResult {
  const Token next = LexerNext(lex);
  return IsLineCommentToken(next, message);
}

DEFINE_TOKEN_ASSERTIONS(BlockComment);

static inline auto IsBlockCommentToken(const Token& lhs, const std::string message) -> testing::AssertionResult {
  if (lhs.kind != kBlockCommentToken)
    return ::testing::AssertionFailure() << "expected token to be a " << kBlockCommentToken << ", but was: " << lhs;

  if (((std::string)lhs.data) != message)
    return ::testing::AssertionFailure() << "expected " << kBlockCommentToken << " to have the message `" << message
                                         << "` but was: " << lhs.data;

  return ::testing::AssertionSuccess();
}

static inline auto IsBlockCommentTokenNext(Lexer* lex, const std::string message) -> testing::AssertionResult {
  const Token next = LexerNext(lex);
  return IsBlockCommentToken(next, message);
}

#undef DEFINE_TOKEN_ASSERTIONS

#endif  // __cplusplus

#endif  // TASKFILE_PARSER_TOKEN_ASSERTIONS_H
