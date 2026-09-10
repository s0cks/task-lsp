#include "lexer.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/token.h"

#define LEXPOS(L)     \
  ((Pos){             \
      .row = L->line, \
      .col = L->col,  \
  })

#define STRVIEW(S)        \
  ((StrView){             \
      .start = strdup(S), \
      .len = strlen(S),   \
  })

#define LEX_STRVIEW_AT(L, Pos)           \
  ((StrView){                            \
      .start = (char*)&lex->source[Pos], \
      .len = strlen(lex->source),        \
  })

#define LEX_STRVIEWN_AT(L, N, Pos)       \
  ((StrView){                            \
      .start = (char*)&lex->source[Pos], \
      .len = N,                          \
  })

#define LEX_STRVIEW1_AT(L, Pos) LEX_STRVIEWN_AT(L, 1, Pos)
#define LEX_STRVIEW2_AT(L, Pos) LEX_STRVIEWN_AT(L, 2, Pos)
#define LEX_STRVIEW3_AT(L, Pos) LEX_STRVIEWN_AT(L, 3, Pos)
#define LEX_STRVIEW4_AT(L, Pos) LEX_STRVIEWN_AT(L, 4, Pos)
#define LEX_STRVIEW5_AT(L, Pos) LEX_STRVIEWN_AT(L, 5, Pos)

#define LEX_STRVIEW(L)          LEX_STRVIEW_AT(L, L->rpos)
#define LEX_STRVIEWN(L, N)      LEX_STRVIEWN_AT(L, N, L->rpos)
#define LEX_STRVIEW1(L)         LEX_STRVIEWN(L, 1)
#define LEX_STRVIEW2(L)         LEX_STRVIEWN(L, 2)
#define LEX_STRVIEW3(L)         LEX_STRVIEWN(L, 3)
#define LEX_STRVIEW4(L)         LEX_STRVIEWN(L, 4)
#define LEX_STRVIEW5(L)         LEX_STRVIEWN(L, 5)

void InitLexer(Lexer* lex, const char* source) {
  ResetLexer(lex);
  lex->source = source;
  lex->rpos = 0;
  lex->line = 1;
  lex->col = 1;
}

static inline char PeekWithOffset(Lexer* lex, const size_t offset) {
  return lex->source[lex->rpos + offset];
}

static inline char Peek(Lexer* lex) {
  return PeekWithOffset(lex, 0);
}

static inline char Advance(Lexer* lex) {
  const char c = lex->source[lex->rpos];
  if (c == '\0')
    return c;
  lex->rpos += 1;
  if (c == '\n') {
    lex->line++;
    lex->col = 1;
  } else {
    lex->col++;
  }

  return c;
}

static inline void SkipWhitespace(Lexer* lex) {
  for (;;) {
    const char c = Peek(lex);
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
      Advance(lex);
      continue;
    }

    break;
  }
}

static inline Token NewTokenAtRange(TokenKind kind, StrView value, Range range) {
  Token tok = {.kind = kind, .data = value, .range = range};
  return tok;
}

static inline Token NewTokenAtPos(TokenKind kind, StrView value, Pos pos) {
  Range range = (Range){.start = pos, .end = pos};
  return NewTokenAtRange(kind, value, range);
}

static Token ErrorToken(Lexer* lex, const char* msg) {
  char buf[256];
  snprintf(buf, sizeof(buf), "%s (line %d, col %d)", msg, lex->line, lex->col);
  return NewTokenAtPos(kInvalidToken, (StrView){.start = strdup(buf), .len = strlen(buf)}, LEXPOS(lex));
}

static inline Token NewInvalidTokenAt(Lexer* lex, Pos pos) {
  return NewTokenAtPos(kInvalidToken, LEX_STRVIEW(lex), LEXPOS(lex));
}

static inline Token LexNumber(Lexer* lex) {
  char buf[64];
  int n = 0;

  while (isdigit((unsigned char)Peek(lex)) || Peek(lex) == '.' || Peek(lex) == '-') {
    if (n >= (int)sizeof(buf) - 1)
      return ErrorToken(lex, "number literal too long");

    buf[n++] = Advance(lex);
  }

  buf[n] = '\0';
  return NewTokenAtPos(kNumberToken, STRVIEW(buf), LEXPOS(lex));
}

static inline Token ParseCommentTokenAt(Lexer* lex, Pos start) {
  StrView empty;
  memset(&empty, 0, sizeof(StrView));

  const size_t startpos = lex->rpos;
  do {
    const char next = Peek(lex);
    switch (next) {
      case '\r':
        Advance(lex);
        continue;
      case '\n':
        if (PeekWithOffset(lex, 1) != '#')
          goto finished;

        Advance(lex);  // skip newline
        Advance(lex);  // skip #
        continue;
      case '\0':
        goto finished;
      default:
        Advance(lex);
    }
  } while (true);
finished:
  const size_t endpos = lex->rpos;
  const size_t len = (endpos - startpos);
  return NewTokenAtPos(kBlockCommentToken, LEX_STRVIEWN_AT(lex, len, startpos), LEXPOS(lex));
}

Token LexerNext(Lexer* lex) {
  StrView empty;
  memset(&empty, 0, sizeof(StrView));
  if (!lex || !lex->source)
    return NewTokenAtPos(kEofToken, empty, LEXPOS(lex));

try_again:
  const size_t rpos = lex->rpos;
  const char c = Peek(lex);
  switch (c) {
    case '\0':
      return NewTokenAtPos(kEofToken, empty, LEXPOS(lex));

    case '\n':
    case '\r':
      while (Peek(lex) == '\r' || Peek(lex) == '\n')
        Advance(lex);
      goto try_again;

    case ':':
      return NewTokenAtPos(kColonToken, LEX_STRVIEW1_AT(lex, rpos), LEXPOS(lex));

    case '>':
      return NewTokenAtPos(kArrowToken, LEX_STRVIEW1_AT(lex, rpos), LEXPOS(lex));

    case '#':
      return ParseCommentTokenAt(lex, LEXPOS(lex));

    case '-':
      int len = 1;
      do {
        Advance(lex);
        if (Peek(lex) != '-')
          break;
        len++;
      } while (len < 4);

      if (len == 3)
        return NewTokenAtPos(kDocumentSeparatorToken, LEX_STRVIEWN_AT(lex, len, rpos), LEXPOS(lex));
      else if (len == 1)
        return NewTokenAtPos(kDashToken, LEX_STRVIEW1_AT(lex, rpos), LEXPOS(lex));
      return NewTokenAtPos(kInvalidToken, LEX_STRVIEWN_AT(lex, len, rpos), LEXPOS(lex));
  }

  return NewTokenAtPos(kInvalidToken, LEX_STRVIEW(lex), LEXPOS(lex));
}

void ResetLexer(Lexer* lex) {
  if (!lex)
    return;
  memset(lex, 0, sizeof(Lexer));
}
