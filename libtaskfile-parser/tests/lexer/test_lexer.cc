#include <gtest/gtest.h>

#include "lexer.h"
#include "seq.h"
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

static inline auto IsLexerFinished(Lexer* lex) -> AssertionResult {
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

  ASSERT_TRUE(IsBlockCommentTokenNext(&lex, "This is a test"));
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

  ASSERT_TRUE(IsBlockCommentTokenNext(&lex, "This is a test\nof a multi-line block comment"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_Tasks) {
  static const auto kTestDocument =
      "tasks:"
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, 0);

  ASSERT_TRUE(IsKeyTokenNext(&lex, "tasks"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_Vars) {
  static const auto kTestDocument =
      "vars:"
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, 0);

  ASSERT_TRUE(IsKeyTokenNext(&lex, "vars"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_Desc_Quoted) {
  static const auto kTestDocument =
      "desc: \"Hello World\""
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, 0);

  ASSERT_TRUE(IsKeyTokenNext(&lex, "desc"));
  ASSERT_TRUE(IsStringTokenNext(&lex, "Hello World"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_Desc_Unquoted) {
  static const auto kTestDocument =
      "desc: Hello World"
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  ASSERT_STREQ(lex.source, kTestDocument);
  ASSERT_EQ(lex.rpos, 0);

  ASSERT_TRUE(IsKeyTokenNext(&lex, "desc"));
  ASSERT_TRUE(IsStringTokenNext(&lex, "Hello World"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_TaskName) {
  static const auto kTestDocument =
      "build:"
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));

  ASSERT_TRUE(IsKeyTokenNext(&lex, "build"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_NegativeNumber) {
  static const auto kTestDocument =
      "PORT: -1"
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));

  ASSERT_TRUE(IsKeyTokenNext(&lex, "PORT"));
  ASSERT_TRUE(IsNumberTokenNext(&lex, "-1"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_HyphenatedWord) {
  static const auto kTestDocument =
      "- co-located"
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));

  ASSERT_TRUE(IsDashTokenNext(&lex));
  ASSERT_TRUE(IsStringTokenNext(&lex, "co-located"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_TrailingComment) {
  static const auto kTestDocument = "build: # trailing\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));

  ASSERT_TRUE(IsKeyTokenNext(&lex, "build"));
  ASSERT_TRUE(IsLineCommentTokenNext(&lex, "trailing"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_Indent_TokenCarriesIndentAndLeadingWhitespace) {
  static const auto kTestDocument =
      "tasks:\n"
      "  build:\n"
      "    cmds:\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));

  const Token tasks = LexerNext(&lex);
  ASSERT_TRUE(IsKeyToken(tasks, "tasks"));
  ASSERT_EQ(tasks.indent, 0);
  ASSERT_EQ(tasks.leading_ws, 0);

  const Token build = LexerNext(&lex);
  ASSERT_TRUE(IsKeyToken(build, "build"));
  ASSERT_EQ(build.indent, 2);
  ASSERT_EQ(build.leading_ws, 2);

  const Token cmds = LexerNext(&lex);
  ASSERT_TRUE(IsKeyToken(cmds, "cmds"));
  ASSERT_EQ(cmds.indent, 4);
  ASSERT_EQ(cmds.leading_ws, 4);
}

TEST_F(TestLexer, Test_Indent_WidthDetectedFromSmallestStep) {
  static const auto kTwoSpace =
      "tasks:\n"
      "  build:\n"
      "    cmds:\n";

  Lexer two;
  memset(&two, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&two, kTwoSpace));
  while (LexerNext(&two).kind != kEofToken) {}
  ASSERT_EQ(LexerIndentWidth(&two), 2);
  ASSERT_FALSE(two.saw_tab_indent);

  static const auto kFourSpace =
      "tasks:\n"
      "    build:\n"
      "        cmds:\n";

  Lexer four;
  memset(&four, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&four, kFourSpace));
  while (LexerNext(&four).kind != kEofToken) {}
  ASSERT_EQ(LexerIndentWidth(&four), 4);
}

TEST_F(TestLexer, Test_Indent_TabsAreFlagged) {
  static const auto kTestDocument =
      "tasks:\n"
      "\tbuild:\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  while (LexerNext(&lex).kind != kEofToken) {}
  ASSERT_TRUE(lex.saw_tab_indent);
}

TEST_F(TestLexer, Test_Meta_NewlineStyleDetection) {
  static const auto kLf = "a: 1\nb: 2\n";
  Lexer lf;
  memset(&lf, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lf, kLf));
  while (LexerNext(&lf).kind != kEofToken) {}
  ASSERT_EQ(lf.newline_style, kLfNewline);

  static const auto kCrLf = "a: 1\r\nb: 2\r\n";
  Lexer crlf;
  memset(&crlf, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&crlf, kCrLf));
  while (LexerNext(&crlf).kind != kEofToken) {}
  ASSERT_EQ(crlf.newline_style, kCrLfNewline);

  static const auto kMixed = "a: 1\nb: 2\r\n";
  Lexer mixed;
  memset(&mixed, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&mixed, kMixed));
  while (LexerNext(&mixed).kind != kEofToken) {}
  ASSERT_EQ(mixed.newline_style, kMixedNewline);
}

TEST_F(TestLexer, Test_Meta_BlankLinesAreRecordedByLineNumber) {
  static const auto kTestDocument =
      "a: 1\n"
      "\n"
      "b: 2\n"
      "   \n"
      "c: 3\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));
  while (LexerNext(&lex).kind != kEofToken) {}

  ASSERT_EQ(lex.blank_lines.len, 2);
  ASSERT_EQ(lex.blank_lines.values[0], 2);
  ASSERT_EQ(lex.blank_lines.values[1], 4);
}

TEST_F(TestLexer, Test_Meta_ScalarStylesOnTokens) {
  static const auto kTestDocument =
      "plain: hello\n"
      "single: 'hello'\n"
      "double: \"hello\"\n"
      "literal: |\n"
      "folded: >-\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));

  ASSERT_TRUE(IsKeyTokenNext(&lex, "plain"));
  ASSERT_EQ(LexerNext(&lex).style, kPlainScalarStyle);

  ASSERT_TRUE(IsKeyTokenNext(&lex, "single"));
  ASSERT_EQ(LexerNext(&lex).style, kSingleQuotedScalarStyle);

  ASSERT_TRUE(IsKeyTokenNext(&lex, "double"));
  ASSERT_EQ(LexerNext(&lex).style, kDoubleQuotedScalarStyle);

  ASSERT_TRUE(IsKeyTokenNext(&lex, "literal"));
  const Token literal = LexerNext(&lex);
  ASSERT_EQ(literal.style, kLiteralBlockScalarStyle);
  ASSERT_EQ(literal.chomp, kClipChomp);

  ASSERT_TRUE(IsKeyTokenNext(&lex, "folded"));
  const Token folded = LexerNext(&lex);
  ASSERT_EQ(folded.style, kFoldedBlockScalarStyle);
  ASSERT_EQ(folded.chomp, kStripChomp);
}

TEST_F(TestLexer, Test_NextToken_UnquotedKeyWithInteriorColon) {
  static const auto kTestDocument =
      "go:build:"
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));

  ASSERT_TRUE(IsKeyTokenNext(&lex, "go:build"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_UnquotedKeyColonNoSpaceIsNotAKey) {
  static const auto kTestDocument =
      "go:build"
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));

  ASSERT_TRUE(IsStringTokenNext(&lex, "go:build"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_QuotedKeyWithColonInName) {
  static const auto kTestDocument =
      "\"docker:build\": 1"
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));

  ASSERT_TRUE(IsKeyTokenNext(&lex, "docker:build"));
  ASSERT_TRUE(IsNumberTokenNext(&lex, "1"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}

TEST_F(TestLexer, Test_NextToken_QuotedValueIsNotAKey) {
  static const auto kTestDocument =
      "deps: \"docker:build\""
      "\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));

  ASSERT_TRUE(IsKeyTokenNext(&lex, "deps"));
  ASSERT_TRUE(IsStringTokenNext(&lex, "docker:build"));
  ASSERT_TRUE(IsLexerFinished(&lex));
}
TEST_F(TestLexer, Test_NextToken_BlockScalarHeader) {
  static const auto kTestDocument =
      "NOTES: |-\n"
      "  hi\n";

  Lexer lex;
  memset(&lex, 0, sizeof(Lexer));
  ASSERT_NO_FATAL_FAILURE(InitLexer(&lex, kTestDocument));

  ASSERT_TRUE(IsKeyTokenNext(&lex, "NOTES"));
  ASSERT_TRUE(IsBlockScalarHeaderTokenNext(&lex, "|-"));
}
