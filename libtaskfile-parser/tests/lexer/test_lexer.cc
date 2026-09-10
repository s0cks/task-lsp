#include <gtest/gtest.h>

#include "gtest/gtest.h"
#include "lexer.h"
#include "token.h"
#include "token_assertions.h"

using namespace ::testing;

class TestLexer : public Test {};

TEST_F(TestLexer, Test_Init_NullSource) {
  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, nullptr));
  ASSERT_EQ(lex.source, nullptr);
  ASSERT_EQ(lex.rpos, 0);
}

TEST_F(TestLexer, Test_Init) {
  static const char* kTestDocument =
      "---\n"
      "message: Hello World\n"
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  ASSERT_NE(lex.source, nullptr);
  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, 0);
}

static inline auto IsLexerFinished(Lexer* lex) -> ::testing::AssertionResult {
  const Token next = LexerNext(lex);
  if (next.kind != kEofToken)
    return AssertionFailure() << "expected next token to be an EOF, but was: " << next;
  const size_t expected_rpos = lex->source ? strlen(lex->source) : 0;
  if (lex->source && lex->rpos != expected_rpos)
    return AssertionFailure() << "expected Lexer to be at " << expected_rpos << " but was at " << lex->rpos;
  return AssertionSuccess();
}

TEST_F(TestLexer, Test_NextToken_EmptySourceEof) {
  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, nullptr));
  ASSERT_EQ(lex.source, nullptr);
  ASSERT_EQ(lex.rpos, 0);
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_Dash) {
  static const auto kTestDocument = "-\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, 0);

  ASSERT_TRUE(IsDashTokenNext(&lex));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_DoubleDash) {
  static const auto kTestDocument = "--\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, 0);

  ASSERT_TRUE(IsInvalidTokenNext(&lex));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_TripleDash) {
  static const auto kTestDocument = "---\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, 0);

  ASSERT_TRUE(IsDocumentSeparatorTokenNext(&lex));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_BlockComment_SingleLine) {
  static const auto kTestDocument =
      "# This is a test\n"
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, 0);

  ASSERT_TRUE(IsBlockCommentTokenNext(&lex, "# This is a test"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_BlockComment_MultiLine) {
  static const auto kTestDocument =
      "# This is a test\n"
      "# of a multi-line block comment\n"
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, 0);

  ASSERT_TRUE(IsBlockCommentTokenNext(&lex, "# This is a test\n# of a multi-line block comment"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}
