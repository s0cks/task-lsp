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

static inline auto IsEofToken(const Token& rhs) -> ::testing::AssertionResult {
  return IsTokenKind(rhs, kEofToken);
}

static inline auto IsEofTokenNext(Lexer* lex) -> ::testing::AssertionResult {
  const Token next = LexerNext(lex);
  return IsEofToken(next);
}

static inline auto IsDashToken(const Token& rhs) -> ::testing::AssertionResult {
  return IsTokenKind(rhs, kDashToken);
}

static inline auto IsDashTokenNext(Lexer* lex) -> ::testing::AssertionResult {
  const Token next = LexerNext(lex);
  return IsDashToken(next);
}

static inline auto IsInvalidToken(const Token& rhs) -> ::testing::AssertionResult {
  return IsTokenKind(rhs, kInvalidToken);
}

static inline auto IsInvalidTokenNext(Lexer* lex) -> ::testing::AssertionResult {
  const Token next = LexerNext(lex);
  return IsInvalidToken(next);
}

static inline auto IsDocumentSeparatorToken(const Token& rhs) -> ::testing::AssertionResult {
  return IsTokenKind(rhs, kDocumentSeparatorToken);
}

static inline auto IsDocumentSeparatorTokenNext(Lexer* lex) -> ::testing::AssertionResult {
  const Token next = LexerNext(lex);
  return IsDocumentSeparatorToken(next);
}

static inline auto IsLineCommentToken(const Token& rhs) -> ::testing::AssertionResult {
  return IsTokenKind(rhs, kLineCommentToken);
}

static inline auto IsLineCommentTokenNext(Lexer* lex) -> ::testing::AssertionResult {
  const Token next = LexerNext(lex);
  return IsLineCommentToken(next);
}

static inline auto IsBlockCommentToken(const Token& rhs) -> ::testing::AssertionResult {
  return IsTokenKind(rhs, kBlockCommentToken);
}

static inline auto IsBlockCommentTokenNext(Lexer* lex) -> ::testing::AssertionResult {
  const Token next = LexerNext(lex);
  return IsBlockCommentToken(next);
}

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

#endif  // __cplusplus

#endif  // TASKFILE_PARSER_TOKEN_ASSERTIONS_H
