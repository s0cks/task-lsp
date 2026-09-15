#include "lexer.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

static inline Pos LexPos(const Lexer* lex) {
  return (Pos){.row = lex->line, .col = lex->col};
}

static inline char PeekAt(const Lexer* lex, const size_t offset) {
  return lex->source[lex->rpos + offset];
}

static inline char Peek(const Lexer* lex) {
  return PeekAt(lex, 0);
}

static inline void RecordNewlineStyle(Lexer* lex, const NewlineStyle seen) {
  if (lex->newline_style == kUnknownNewline) {
    lex->newline_style = seen;
    return;
  }
  if (lex->newline_style != seen)
    lex->newline_style = kMixedNewline;
}

static inline char Advance(Lexer* lex) {
  const char c = lex->source[lex->rpos];
  if (c == '\0')
    return c;

  lex->rpos++;
  if (c == '\n') {
    const bool crlf = lex->rpos >= 2 && lex->source[lex->rpos - 2] == '\r';
    RecordNewlineStyle(lex, crlf ? kCrLfNewline : kLfNewline);
    lex->line++;
    lex->col = 1;
    lex->line_has_content = false;
  } else {
    lex->col++;
  }
  return c;
}

static inline bool AtEnd(const Lexer* lex) {
  return Peek(lex) == '\0';
}

static inline bool AtEndOfLine(const Lexer* lex) {
  return AtEnd(lex) || Peek(lex) == '\n' || Peek(lex) == '\r';
}

static inline int SkipInlineSpace(Lexer* lex) {
  const int begin_col = lex->col;
  while (Peek(lex) == ' ' || Peek(lex) == '\t') {
    if (Peek(lex) == '\t' && !lex->line_has_content)
      lex->saw_tab_indent = true;
    Advance(lex);
  }
  return lex->col - begin_col;
}

static void RecordIndentStep(Lexer* lex, const int indent) {
  if (indent > lex->last_indent) {
    const int step = indent - lex->last_indent;
    if (lex->min_indent_step == 0 || step < lex->min_indent_step)
      lex->min_indent_step = step;
  }
  lex->last_indent = indent;
}

static void RecordBlankLine(Lexer* lex, const int line) {
  const size_t new_len = lex->blank_lines.len + 1;
  if (new_len > lex->blank_lines.cap) {
    size_t new_cap = lex->blank_lines.cap == 0 ? 8 : lex->blank_lines.cap;
    while (new_cap < new_len)
      new_cap *= 2;
    lex->blank_lines.values = (int*)realloc(lex->blank_lines.values, sizeof(int) * new_cap);
    lex->blank_lines.cap = new_cap;
  }
  lex->blank_lines.values[lex->blank_lines.len] = line;
  lex->blank_lines.len = new_len;
}

int LexerIndentWidth(const Lexer* lex) {
  return lex && lex->min_indent_step > 0 ? lex->min_indent_step : 2;
}

static inline bool IsKeyChar(const char c) {
  return isalnum((unsigned char)c) || c == '_' || c == '.' || c == '$' || c == '-';
}

static inline Token NewTokenAtRange(const TokenKind kind, const StrView value, const Range range) {
  return (Token){.kind = kind, .data = value, .range = range, .style = kNoneScalarStyle, .chomp = kDefaultChomp};
}

static inline Token NewStyledToken(const TokenKind kind, const StrView value, const Range range,
                                   const ScalarStyle style) {
  Token tok = NewTokenAtRange(kind, value, range);
  tok.style = style;
  return tok;
}

static inline Token NewTokenAtPos(const TokenKind kind, const StrView value, const Pos pos) {
  return NewTokenAtRange(kind, value, (Range){.start = pos, .end = pos});
}

static inline StrView ViewAt(Lexer* lex, const size_t start, const size_t len) {
  return (StrView){.start = (char*)&lex->source[start], .len = len};
}

void InitLexer(Lexer* lex, const char* source) {
  ResetLexer(lex);
  lex->source = source;
  lex->rpos = 0;
  lex->line = 1;
  lex->col = 1;
  lex->line_has_content = false;
}

void ResetLexer(Lexer* lex) {
  if (!lex)
    return;
  memset(lex, 0, sizeof(Lexer));
}

static Token LexNumber(Lexer* lex) {
  const Pos start = LexPos(lex);
  const size_t begin = lex->rpos;
  if (Peek(lex) == '-')
    Advance(lex);
  while (isdigit((unsigned char)Peek(lex)) || Peek(lex) == '.')
    Advance(lex);
  lex->line_has_content = true;
  return NewTokenAtRange(kNumberToken, ViewAt(lex, begin, lex->rpos - begin), (Range){start, LexPos(lex)});
}

static char* UnescapeDoubleQuoted(const char* start, const size_t len, size_t* out_len) {
  char* buf = (char*)malloc(len + 1);
  size_t w = 0;
  for (size_t i = 0; i < len; i++) {
    if (start[i] == '\\' && i + 1 < len) {
      i++;
      switch (start[i]) {
        case 'n':
          buf[w++] = '\n';
          break;
        case 't':
          buf[w++] = '\t';
          break;
        case 'r':
          buf[w++] = '\r';
          break;
        case '"':
          buf[w++] = '"';
          break;
        case '\\':
          buf[w++] = '\\';
          break;
        default:
          buf[w++] = start[i];
      }
      continue;
    }
    buf[w++] = start[i];
  }
  buf[w] = '\0';
  *out_len = w;
  return buf;
}

static char* UnescapeSingleQuoted(const char* start, const size_t len, size_t* out_len) {
  char* buf = (char*)malloc(len + 1);
  size_t w = 0;
  for (size_t i = 0; i < len; i++) {
    if (start[i] == '\'' && i + 1 < len && start[i + 1] == '\'') {
      buf[w++] = '\'';
      i++;
      continue;
    }
    buf[w++] = start[i];
  }
  buf[w] = '\0';
  *out_len = w;
  return buf;
}

static Token LexQuotedString(Lexer* lex);

static bool LooksLikeQuotedKey(const Lexer* lex) {
  const char quote = Peek(lex);
  size_t i = 1;
  while (PeekAt(lex, i) != quote && PeekAt(lex, i) != '\0' && PeekAt(lex, i) != '\n') {
    if (quote == '"' && PeekAt(lex, i) == '\\' && PeekAt(lex, i + 1) != '\0')
      i++;
    i++;
  }
  if (PeekAt(lex, i) != quote)
    return false;
  i++;
  if (PeekAt(lex, i) != ':')
    return false;
  const char after = PeekAt(lex, i + 1);
  return after == '\0' || after == ' ' || after == '\n' || after == '\r';
}

static Token LexQuotedKey(Lexer* lex) {
  const Token str = LexQuotedString(lex);
  Advance(lex);  // consume ':'
  lex->line_has_content = true;
  return NewTokenAtRange(kKeyToken, str.data, (Range){str.range.start, LexPos(lex)});
}

static Token LexQuotedString(Lexer* lex) {
  const Pos start = LexPos(lex);
  const char quote = Advance(lex);
  const size_t begin = lex->rpos;

  while (!AtEndOfLine(lex) && Peek(lex) != quote) {
    if (quote == '"' && Peek(lex) == '\\' && !AtEnd(lex))
      Advance(lex);
    Advance(lex);
  }

  const size_t len = lex->rpos - begin;
  if (Peek(lex) == quote)
    Advance(lex);

  size_t unescaped_len = 0;
  char* unescaped = quote == '"' ? UnescapeDoubleQuoted(lex->source + begin, len, &unescaped_len)
                                 : UnescapeSingleQuoted(lex->source + begin, len, &unescaped_len);

  lex->line_has_content = true;
  return NewStyledToken(kStringToken, (StrView){.start = unescaped, .len = unescaped_len}, (Range){start, LexPos(lex)},
                        quote == '"' ? kDoubleQuotedScalarStyle : kSingleQuotedScalarStyle);
}

static bool LooksLikeBlockScalarHeader(const Lexer* lex) {
  size_t i = 1;
  if (PeekAt(lex, i) == '-' || PeekAt(lex, i) == '+')
    i++;
  while (isdigit((unsigned char)PeekAt(lex, i)))
    i++;
  while (PeekAt(lex, i) == ' ')
    i++;
  const char c = PeekAt(lex, i);
  return c == '\0' || c == '\n' || c == '\r' || c == '#';
}

static Token LexBlockScalarHeader(Lexer* lex) {
  const Pos start = LexPos(lex);
  const size_t begin = lex->rpos;
  const char marker = Peek(lex);
  Advance(lex);

  ChompStyle chomp = kDefaultChomp;
  while (Peek(lex) == '-' || Peek(lex) == '+' || isdigit((unsigned char)Peek(lex))) {
    if (Peek(lex) == '-')
      chomp = kStripChomp;
    else if (Peek(lex) == '+')
      chomp = kKeepChomp;
    Advance(lex);
  }

  lex->line_has_content = true;
  Token tok =
      NewStyledToken(kBlockScalarHeaderToken, ViewAt(lex, begin, lex->rpos - begin), (Range){start, LexPos(lex)},
                     marker == '|' ? kLiteralBlockScalarStyle : kFoldedBlockScalarStyle);
  tok.chomp = chomp;
  return tok;
}

static bool LooksLikeKey(const Lexer* lex) {
  size_t i = 0;
  if (!IsKeyChar(PeekAt(lex, i)))
    return false;

  for (;;) {
    while (IsKeyChar(PeekAt(lex, i)))
      i++;
    if (PeekAt(lex, i) != ':')
      return false;
    const char after = PeekAt(lex, i + 1);
    if (after == '\0' || after == ' ' || after == '\n' || after == '\r')
      return true;
    if (!IsKeyChar(after))
      return false;
    i++;  // colon is interior to the key (e.g. "go:build:"); keep scanning
  }
}

static Token LexKey(Lexer* lex) {
  const Pos start = LexPos(lex);
  const size_t begin = lex->rpos;
  for (;;) {
    while (IsKeyChar(Peek(lex)))
      Advance(lex);
    const char after = PeekAt(lex, 1);
    if (Peek(lex) == ':' && after != '\0' && after != ' ' && after != '\n' && after != '\r') {
      Advance(lex);  // consume interior ':', keep scanning the rest of the key
      continue;
    }
    break;
  }
  const StrView name = ViewAt(lex, begin, lex->rpos - begin);
  Advance(lex);  // consume the terminating ':'
  lex->line_has_content = true;
  return NewTokenAtRange(kKeyToken, name, (Range){start, LexPos(lex)});
}

static Token LexPlainScalar(Lexer* lex) {
  const Pos start = LexPos(lex);
  const size_t begin = lex->rpos;
  while (!AtEndOfLine(lex)) {
    if (Peek(lex) == ' ' && PeekAt(lex, 1) == '#')
      break;
    Advance(lex);
  }
  size_t end = lex->rpos;
  while (end > begin && (lex->source[end - 1] == ' ' || lex->source[end - 1] == '\t'))
    end--;
  lex->line_has_content = true;
  return NewStyledToken(kStringToken, ViewAt(lex, begin, end - begin), (Range){start, LexPos(lex)}, kPlainScalarStyle);
}

static Token LexComment(Lexer* lex) {
  const Pos start = LexPos(lex);
  const bool trailing = lex->line_has_content;
  Advance(lex);  // consume '#'
  if (Peek(lex) == ' ')
    Advance(lex);

  size_t begin = lex->rpos;
  while (!AtEndOfLine(lex))
    Advance(lex);
  size_t end = lex->rpos;
  while (end > begin && lex->source[end - 1] == '\r')
    end--;

  if (trailing || AtEnd(lex)) {
    lex->line_has_content = true;
    return NewTokenAtRange(trailing ? kLineCommentToken : kBlockCommentToken, ViewAt(lex, begin, end - begin),
                           (Range){start, LexPos(lex)});
  }

  size_t cap = 128;
  size_t out_len = end - begin;
  char* out = (char*)malloc(cap);
  memcpy(out, lex->source + begin, out_len);

  for (;;) {
    size_t la = 0;
    while (PeekAt(lex, la) == '\r')
      la++;
    if (PeekAt(lex, la) != '\n')
      break;
    la++;
    while (PeekAt(lex, la) == ' ')
      la++;
    if (PeekAt(lex, la) != '#')
      break;

    while (la > 0) {
      Advance(lex);
      la--;
    }
    Advance(lex);  // consume '#'
    if (Peek(lex) == ' ')
      Advance(lex);

    const size_t line_begin = lex->rpos;
    while (!AtEndOfLine(lex))
      Advance(lex);
    size_t line_end = lex->rpos;
    while (line_end > line_begin && lex->source[line_end - 1] == '\r')
      line_end--;
    const size_t line_len = line_end - line_begin;

    if (out_len + 1 + line_len + 1 > cap) {
      while (out_len + 1 + line_len + 1 > cap)
        cap *= 2;
      out = (char*)realloc(out, cap);
    }
    out[out_len++] = '\n';
    memcpy(out + out_len, lex->source + line_begin, line_len);
    out_len += line_len;

    if (AtEnd(lex))
      break;
  }

  lex->line_has_content = true;
  return NewTokenAtRange(kBlockCommentToken, (StrView){.start = out, .len = out_len}, (Range){start, LexPos(lex)});
}

static Token LexerNextRaw(Lexer* lex) {
  static const StrView kEmpty = {0};
  if (!lex || !lex->source)
    return NewTokenAtPos(kEofToken, kEmpty, lex ? LexPos(lex) : (Pos){0, 0});

  for (;;) {
    const bool at_line_start = !lex->line_has_content;
    const int skipped = SkipInlineSpace(lex);
    if (at_line_start)
      lex->leading_ws = skipped;

    if (AtEnd(lex))
      return NewTokenAtPos(kEofToken, kEmpty, LexPos(lex));

    if (Peek(lex) == '\n' || Peek(lex) == '\r') {
      if (at_line_start)
        RecordBlankLine(lex, lex->line);
      if (Peek(lex) == '\r')
        Advance(lex);
      if (Peek(lex) == '\n')
        Advance(lex);
      continue;
    }

    if (at_line_start)
      RecordIndentStep(lex, skipped);

    const char c = Peek(lex);

    if (c == '#')
      return LexComment(lex);

    if (c == '"' || c == '\'')
      return LooksLikeQuotedKey(lex) ? LexQuotedKey(lex) : LexQuotedString(lex);

    if ((c == '|' || c == '>') && LooksLikeBlockScalarHeader(lex))
      return LexBlockScalarHeader(lex);

    if (c == '-') {
      const char after = PeekAt(lex, 1);
      if (isdigit((unsigned char)after) || after == '.')
        return LexNumber(lex);

      int len = 0;
      while (PeekAt(lex, len) == '-')
        len++;
      const char boundary = PeekAt(lex, len);
      const bool clean_boundary = boundary == '\0' || boundary == ' ' || boundary == '\n' || boundary == '\r';

      if (len == 3 && clean_boundary) {
        const Pos start = LexPos(lex);
        const size_t begin = lex->rpos;
        Advance(lex);
        Advance(lex);
        Advance(lex);
        lex->line_has_content = true;
        return NewTokenAtRange(kDocumentSeparatorToken, ViewAt(lex, begin, 3), (Range){start, LexPos(lex)});
      }
      if (len == 1 && clean_boundary) {
        const Pos start = LexPos(lex);
        const size_t begin = lex->rpos;
        Advance(lex);
        lex->line_has_content = true;
        return NewTokenAtRange(kDashToken, ViewAt(lex, begin, 1), (Range){start, LexPos(lex)});
      }
      if (len > 1 && !clean_boundary) {
        return LexPlainScalar(lex);
      }

      const Pos start = LexPos(lex);
      const size_t begin = lex->rpos;
      for (int i = 0; i < len; i++)
        Advance(lex);
      lex->line_has_content = true;
      return NewTokenAtRange(kInvalidToken, ViewAt(lex, begin, (size_t)len), (Range){start, LexPos(lex)});
    }

    if (isdigit((unsigned char)c))
      return LexNumber(lex);

    if (IsKeyChar(c))
      return LooksLikeKey(lex) ? LexKey(lex) : LexPlainScalar(lex);

    return LexPlainScalar(lex);
  }
}

StrView LexerConsumeBlockScalar(Lexer* lex, const int header_indent, const char chomp) {
  if (Peek(lex) == '\r')
    Advance(lex);
  if (Peek(lex) == '\n')
    Advance(lex);

  size_t cap = 64;
  size_t out_len = 0;
  char* out = (char*)malloc(cap);
  int content_indent = -1;
  bool consumed_any_line = false;

  for (;;) {
    if (AtEnd(lex))
      break;

    const size_t line_start = lex->rpos;
    int indent = 0;
    while (PeekAt(lex, indent) == ' ')
      indent++;

    const char after = PeekAt(lex, indent);
    const bool blank_line = after == '\n' || after == '\r' || after == '\0';

    if (!blank_line && indent <= header_indent)
      break;

    if (content_indent < 0 && !blank_line)
      content_indent = indent;
    const int strip = content_indent < 0 ? indent : content_indent;
    const int skip = blank_line ? indent : (strip < indent ? strip : indent);

    for (int i = 0; i < skip; i++)
      Advance(lex);
    ASSERT_EQ(lex->rpos, line_start + (size_t)skip);

    const size_t text_begin = lex->rpos;
    while (!AtEndOfLine(lex))
      Advance(lex);
    const size_t text_len = lex->rpos - text_begin;

    if (out_len + text_len + 1 > cap) {
      while (out_len + text_len + 1 > cap)
        cap *= 2;
      out = (char*)realloc(out, cap);
    }
    memcpy(out + out_len, lex->source + text_begin, text_len);
    out_len += text_len;
    consumed_any_line = true;

    if (AtEnd(lex))
      break;

    Advance(lex);
    if (out_len + 1 > cap) {
      cap *= 2;
      out = (char*)realloc(out, cap);
    }
    out[out_len++] = '\n';
  }

  if (chomp == '-') {
    while (out_len > 0 && out[out_len - 1] == '\n')
      out_len--;
  } else if (chomp != '+') {
    while (out_len > 0 && out[out_len - 1] == '\n')
      out_len--;
    if (consumed_any_line) {
      if (out_len + 1 > cap) {
        cap *= 2;
        out = (char*)realloc(out, cap);
      }
      out[out_len++] = '\n';
    }
  }

  return (StrView){.start = out, .len = out_len};
}

Token LexerNext(Lexer* lex) {
  Token tok = LexerNextRaw(lex);
  tok.indent = tok.range.start.col - 1;
  tok.leading_ws = lex ? lex->leading_ws : 0;
  return tok;
}
