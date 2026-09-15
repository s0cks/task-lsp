#ifndef TASKFILE_PARSER_LEXER_H
#define TASKFILE_PARSER_LEXER_H

#include "taskfile_parser.h"
#include "token.h"

// NOLINTBEGIN(modernize-use-using,modernize-use-trailing-return-type)
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct {
  const char* source;
  Token last_token;
  Token curr_token;
  size_t rpos;
  int line;
  int col;
  bool line_has_content;

  NewlineStyle newline_style;
  bool saw_tab_indent;
  int min_indent_step;
  int last_indent;
  LineSeq blank_lines;
  int leading_ws;
} Lexer;

void InitLexer(Lexer* lex, const char* source);
Token LexerNext(Lexer* lex);
void ResetLexer(Lexer* lex);

StrView LexerConsumeBlockScalar(Lexer* lex, int header_indent, char chomp);

int LexerIndentWidth(const Lexer* lex);

#ifdef __cplusplus
};
#endif  // __cplusplus
// NOLINTEND(modernize-use-using,modernize-use-trailing-return-type)

#endif  // TASKFILE_PARSER_LEXER_H
