#include <gtest/gtest.h>

#include "lexer.h"
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

TEST_F(TestLexer, Test_NextToken_EmptySourceEof) {
  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, nullptr));
  ASSERT_EQ(lex.source, nullptr);
  ASSERT_EQ(lex.rpos, 0);

  Token next = LexerNext(&lex);
  ASSERT_TRUE(IsEofToken(next));
  ASSERT_EQ(lex.source, nullptr);
  ASSERT_EQ(lex.rpos, 0);
}

TEST_F(TestLexer, Test_NextToken_Dash) {
  static const auto kTestDocument = "-\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, 0);

  Token next = LexerNext(&lex);
  ASSERT_TRUE(IsDashToken(next));

  {
    Token next = LexerNext(&lex);
    ASSERT_TRUE(IsEofToken(next));
  }

  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, strlen(kTestDocument));
}

TEST_F(TestLexer, Test_NextToken_DoubleDash) {
  static const auto kTestDocument = "--\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, 0);

  Token next = LexerNext(&lex);
  ASSERT_TRUE(IsInvalidToken(next));

  {
    Token next = LexerNext(&lex);
    ASSERT_TRUE(IsEofToken(next));
  }

  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, strlen(kTestDocument));
}

TEST_F(TestLexer, Test_NextToken_TripleDash) {
  static const auto kTestDocument = "---\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, 0);

  Token next = LexerNext(&lex);
  ASSERT_TRUE(IsDocumentSeparatorToken(next));

  {
    Token next = LexerNext(&lex);
    ASSERT_TRUE(IsEofToken(next));
  }

  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, strlen(kTestDocument));
}

TEST_F(TestLexer, Test_NextToken_SingleLineComment) {
  static const auto kTestDocument =
      "# This is a test\n"
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, 0);

  {
    Token next = LexerNext(&lex);
    ASSERT_TRUE(IsBlockCommentToken(next));
  }

  {
    Token next = LexerNext(&lex);
    ASSERT_TRUE(IsEofToken(next));
  }

  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, strlen(kTestDocument));
}
