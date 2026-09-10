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
} Lexer;

void InitLexer(Lexer* lex, const char* source);
Token LexerNext(Lexer* lex);
void ResetLexer(Lexer* lex);

#ifdef __cplusplus
};
#endif  // __cplusplus
// NOLINTEND(modernize-use-using,modernize-use-trailing-return-type)

#endif  // TASKFILE_PARSER_LEXER_H
