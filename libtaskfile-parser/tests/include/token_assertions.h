#ifndef TASKFILE_PARSER_TOKEN_ASSERTIONS_H
#define TASKFILE_PARSER_TOKEN_ASSERTIONS_H

#include "gtest/gtest.h"
#ifdef __cplusplus

#include <gtest/gtest.h>

#include "token.h"

static inline auto IsTokenKind(const Token& lhs, const TokenKind rhs) -> ::testing::AssertionResult {
  if (lhs.kind != rhs)
    return ::testing::AssertionFailure() << "expected " << lhs << " to be a " << rhs << ", but was a " << lhs.kind;
  return ::testing::AssertionSuccess();
}

static inline auto IsEofToken(const Token& rhs) -> ::testing::AssertionResult {
  return IsTokenKind(rhs, kEofToken);
}

static inline auto IsDashToken(const Token& rhs) -> ::testing::AssertionResult {
  return IsTokenKind(rhs, kDashToken);
}

static inline auto IsInvalidToken(const Token& rhs) -> ::testing::AssertionResult {
  return IsTokenKind(rhs, kInvalidToken);
}

static inline auto IsDocumentSeparatorToken(const Token& rhs) -> ::testing::AssertionResult {
  return IsTokenKind(rhs, kDocumentSeparatorToken);
}

static inline auto IsLineCommentToken(const Token& rhs) -> ::testing::AssertionResult {
  return IsTokenKind(rhs, kLineCommentToken);
}

static inline auto IsBlockCommentToken(const Token& rhs) -> ::testing::AssertionResult {
  return IsTokenKind(rhs, kBlockCommentToken);
}

#endif  // __cplusplus

#endif  // TASKFILE_PARSER_TOKEN_ASSERTIONS_H
