#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "document.h"
#include "lexer.h"
#include "seq.h"

#define PARSE_ERROR(Format, ...)                                         \
  ({                                                                     \
    const size_t message_len = snprintf(NULL, 0, Format, ##__VA_ARGS__); \
    ASSERT_GT(message_len, 0);                                           \
    const size_t total_size = message_len + 1;                           \
    char* message = (char*)calloc(sizeof(char), total_size);             \
    snprintf(message, total_size, Format, ##__VA_ARGS__);                \
    return (TaskfileParseResult){                                        \
        .success = false,                                                \
        .msg = message,                                                  \
    };                                                                   \
  })

typedef enum {
  kFrameRoot,
  kFrameTasks,
  kFrameTaskBody,
  kFrameDepsList,
  kFrameCmdsList,
  kFrameVars,
  kFrameEnv,
  kFrameStringList,
  kFrameStatusList,
  kFramePreconditionList,
  kFramePreconditionBody,
  kFrameRequires,
  kFrameCmdBody,
  kFrameForBody,
  kFrameForMatrix,
  kFrameOutput,
  kFrameIncludeBody,
  kFrameVarBody,
  kFrameMapBody,
  kFrameIncludes,
  kFrameOther,
} FrameKind;

typedef struct {
  int indent;
  FrameKind kind;
  TaskNode* task;
  VarNode* var;
  StringSeq* strings;
  PreconditionNode* precondition;
  CommandNode* command;
  ForNode* for_each;
  IncludeNode* include;
} Frame;

typedef struct {
  Lexer lexer;
  Document* doc;

  Token lookahead;
  bool has_lookahead;

  Frame* stack;
  size_t stack_len;
  size_t stack_cap;

  bool has_pending_comment;
  Token pending_comment;
} Parser;

static inline Token NextTok(Parser* p) {
  if (p->has_lookahead) {
    p->has_lookahead = false;
    return p->lookahead;
  }
  return LexerNext(&p->lexer);
}

static inline Token PeekTok(Parser* p) {
  if (!p->has_lookahead) {
    p->lookahead = LexerNext(&p->lexer);
    p->has_lookahead = true;
  }
  return p->lookahead;
}

static inline int LineIndent(const Token tok) {
  return tok.range.start.col - 1;
}

static inline Frame* TopFrame(Parser* p) {
  ASSERT_GT(p->stack_len, 0);
  return &p->stack[p->stack_len - 1];
}

static inline void PushFrame(Parser* p, const Frame frame) {
  if (p->stack_len + 1 > p->stack_cap) {
    p->stack_cap = p->stack_cap == 0 ? 8 : p->stack_cap * 2;
    p->stack = (Frame*)realloc(p->stack, sizeof(Frame) * p->stack_cap);
  }
  p->stack[p->stack_len++] = frame;
}

static inline void PopFramesTo(Parser* p, const int indent) {
  while (p->stack_len > 1 && indent <= TopFrame(p)->indent)
    p->stack_len--;
}

static inline void FlushPendingCommentAsFloating(Parser* p) {
  if (!p->has_pending_comment)
    return;

  CommentNode comment = {0};
  comment.kind = kCommentKind;
  comment.start = p->pending_comment.range.start;
  comment.end = p->pending_comment.range.end;
  comment.value = p->pending_comment.data;

  CommentNode* slot = AppendNewCommentInSeq(&p->doc->comments);
  *slot = comment;
  p->has_pending_comment = false;
}

static inline void QueuePendingComment(Parser* p, const Token tok) {
  FlushPendingCommentAsFloating(p);
  p->pending_comment = tok;
  p->has_pending_comment = true;
}

static inline void AttachOrFlushPendingComment(Parser* p, DocumentNode* node, const int row) {
  if (!p->has_pending_comment)
    return;

  if (row != p->pending_comment.range.end.row + 1) {
    FlushPendingCommentAsFloating(p);
    return;
  }

  CommentNode comment = {0};
  comment.kind = kCommentKind;
  comment.start = p->pending_comment.range.start;
  comment.end = p->pending_comment.range.end;
  comment.value = p->pending_comment.data;
  AddCommentToNode(node, comment);
  p->has_pending_comment = false;
}

static inline void MaybeAttachTrailingComment(Parser* p, DocumentNode* node, const int row) {
  const Token peek = PeekTok(p);
  if (peek.kind != kLineCommentToken || peek.range.start.row != row)
    return;
  NextTok(p);

  CommentNode* comment = NewCommentNode();
  comment->start = peek.range.start;
  comment->end = peek.range.end;
  comment->value = peek.data;
  comment->indent = peek.indent;
  comment->leading_ws = peek.leading_ws;

  node->trailing_comment = comment;
  node->trailing_ws = peek.leading_ws;
}

static inline StrView TrimQuotes(const StrView v) {
  if (v.len >= 2 &&
      ((v.start[0] == '"' && v.start[v.len - 1] == '"') || (v.start[0] == '\'' && v.start[v.len - 1] == '\'')))
    return (StrView){.start = v.start + 1, .len = v.len - 2};
  return v;
}

static inline StrView RTrimView(StrView v) {
  while (v.len > 0 && v.start[v.len - 1] == ' ')
    v.len--;
  return v;
}

static inline StrView LTrimView(StrView v) {
  while (v.len > 0 && v.start[0] == ' ') {
    v.start++;
    v.len--;
  }
  return v;
}

static inline StrView TrimView(const StrView v) {
  return RTrimView(LTrimView(v));
}

static inline bool IsIdentChar(const char c) {
  return isalnum((unsigned char)c) || c == '_';
}

static inline Pos MaxPos(const Pos a, const Pos b) {
  if (a.row != b.row)
    return a.row > b.row ? a : b;
  return a.col > b.col ? a : b;
}

static inline Pos PositionAtOffset(const Pos base, const StrView text, const size_t offset) {
  Pos pos = base;
  const size_t limit = offset < text.len ? offset : text.len;
  for (size_t k = 0; k < limit; k++) {
    if (text.start[k] == '\n') {
      pos.row++;
      pos.col = 1;
    } else {
      pos.col++;
    }
  }
  return pos;
}

static inline void StampKeyedMeta(Parser* p, DocumentNode* node, const Token key_tok, const Token value_tok,
                                  const bool has_value) {
  node->key_range = key_tok.range;
  node->indent = key_tok.indent;
  node->leading_ws = key_tok.leading_ws;
  node->depth = (int)p->stack_len - 1;
  if (has_value)
    node->value_range = value_tok.range;
}

static inline void StampScalarMeta(DocumentNode* node, const Token tok) {
  node->style = tok.style;
  node->value_range = tok.range;
  node->indent = tok.indent;
}

static inline DocumentNode* ClassifyScalarValue(const Token tok) {
  if (tok.kind == kNumberToken) {
    NumberNode* n = (NumberNode*)calloc(1, sizeof(NumberNode));
    n->kind = kNumberKind;
    StampScalarMeta((DocumentNode*)n, tok);
    n->start = tok.range.start;
    n->end = tok.range.end;
    n->raw = tok.data;
    n->value = tok.data.start ? strtod(tok.data.start, NULL) : 0.0;
    return (DocumentNode*)n;
  }

  if (StrViewEqualsCStr(tok.data, "true") || StrViewEqualsCStr(tok.data, "false")) {
    BoolNode* b = (BoolNode*)calloc(1, sizeof(BoolNode));
    b->kind = kBoolKind;
    StampScalarMeta((DocumentNode*)b, tok);
    b->start = tok.range.start;
    b->end = tok.range.end;
    b->value = StrViewEqualsCStr(tok.data, "true");
    return (DocumentNode*)b;
  }

  if (tok.data.len == 0 || StrViewEqualsCStr(tok.data, "null") || StrViewEqualsCStr(tok.data, "~")) {
    NullNode* n = (NullNode*)calloc(1, sizeof(NullNode));
    n->kind = kNullKind;
    StampScalarMeta((DocumentNode*)n, tok);
    n->start = tok.range.start;
    n->end = tok.range.end;
    return (DocumentNode*)n;
  }

  StringNode* s = (StringNode*)calloc(1, sizeof(StringNode));
  s->kind = kStringKind;
  StampScalarMeta((DocumentNode*)s, tok);
  s->start = tok.range.start;
  s->end = tok.range.end;
  s->value = tok.data;
  return (DocumentNode*)s;
}

static inline void ParsePipelineInto(const StrView body, RefNode* ref, const StrView text, const Pos text_start) {
  const size_t body_offset = (size_t)(body.start - text.start);
  size_t i = 0;
  while (i < body.len) {
    while (i < body.len && (body.start[i] == ' ' || body.start[i] == '-'))
      i++;
    if (i >= body.len || body.start[i] != '|')
      return;
    i++;
    while (i < body.len && body.start[i] == ' ')
      i++;

    const size_t name_start = i;
    while (i < body.len && IsIdentChar(body.start[i]))
      i++;
    if (i == name_start)
      return;
    const StrView name = {.start = body.start + name_start, .len = i - name_start};

    while (i < body.len && body.start[i] == ' ')
      i++;
    const size_t args_start = i;
    bool in_quote = false;
    char quote_char = 0;
    size_t stage_end = i;
    while (stage_end < body.len) {
      const char c = body.start[stage_end];
      if (in_quote) {
        if (c == quote_char)
          in_quote = false;
      } else if (c == '"' || c == '\'') {
        in_quote = true;
        quote_char = c;
      } else if (c == '|') {
        break;
      }
      stage_end++;
    }
    const StrView args = RTrimView((StrView){.start = body.start + args_start, .len = stage_end - args_start});

    PipelineExprNode* expr = AppendNewPipelineExprInSeq(&ref->pipeline);
    expr->kind = kPipelineExprKind;
    expr->start = PositionAtOffset(text_start, text, body_offset + name_start);
    expr->end = PositionAtOffset(text_start, text, body_offset + stage_end);
    ref->end = expr->end;
    expr->name = name;
    expr->args = args;
    i = stage_end;
  }
}

static inline void ScanTemplateRefsInto(StringNode* owner, const StrView text, const Pos text_start) {
  if (StrViewIsEmpty(text))
    return;

  const char* s = text.start;
  const size_t n = text.len;
  size_t i = 0;
  while (i + 1 < n) {
    if (!(s[i] == '{' && s[i + 1] == '{')) {
      i++;
      continue;
    }

    size_t j = i + 2;
    while (j < n && (s[j] == '-' || s[j] == ' '))
      j++;
    if (j >= n || s[j] != '.') {
      i = j;
      continue;
    }
    j++;

    const size_t name_start = j;
    while (j < n && IsIdentChar(s[j]))
      j++;
    if (j == name_start) {
      i = j;
      continue;
    }
    const StrView name = {.start = (char*)s + name_start, .len = j - name_start};

    size_t end = n;
    for (size_t m = j; m + 1 < n; m++) {
      if (s[m] == '}' && s[m + 1] == '}') {
        end = m;
        break;
      }
    }
    const StrView body = TrimView((StrView){.start = (char*)s + j, .len = (end <= n ? end : n) - j});

    RefNode* ref = AppendNewRefInSeq(&owner->refs);
    ref->kind = kRefKind;
    ref->ref_kind = kVarRefKind;
    ref->name = name;
    ref->start = PositionAtOffset(text_start, text, name_start);
    ref->end = PositionAtOffset(text_start, text, j);
    ref->from = (DocumentNode*)owner;
    ParsePipelineInto(body, ref, text, text_start);

    i = end < n ? end + 2 : n;
  }
}

static inline void ScanEnvRefsInto(StringNode* owner, const StrView text, const Pos text_start) {
  if (StrViewIsEmpty(text))
    return;

  const char* s = text.start;
  const size_t n = text.len;
  size_t i = 0;
  while (i < n) {
    if (s[i] != '$' || (i > 0 && s[i - 1] == '$')) {
      i++;
      continue;
    }

    const bool braced = i + 1 < n && s[i + 1] == '{';
    size_t name_start = i + 1 + (braced ? 1 : 0);
    size_t j = name_start;
    while (j < n && (IsIdentChar(s[j]))) {
      if (j == name_start && isdigit((unsigned char)s[j]))
        break;
      j++;
    }
    if (j == name_start) {
      i++;
      continue;
    }

    size_t end = j;
    if (braced) {
      if (j >= n || s[j] != '}') {
        i = j;
        continue;
      }
      end = j + 1;
    }

    const StrView name = {.start = (char*)s + name_start, .len = j - name_start};
    RefNode* ref = AppendNewRefInSeq(&owner->refs);
    ref->kind = kRefKind;
    ref->ref_kind = kEnvRefKind;
    ref->name = name;
    ref->start = PositionAtOffset(text_start, text, name_start);
    ref->end = PositionAtOffset(text_start, text, j);
    ref->from = (DocumentNode*)owner;

    i = end;
  }
}

static inline void ScanRefsInto(StringNode* owner, const StrView text, const Pos text_start) {
  ScanTemplateRefsInto(owner, text, text_start);
  ScanEnvRefsInto(owner, text, text_start);
}

static inline StringNode BuildStyledStringNode(const StrView value, const Pos start, const Pos end,
                                               const ScalarStyle style) {
  StringNode s = {0};
  s.kind = kStringKind;
  s.start = start;
  s.end = end;
  s.value_range = (Range){start, end};
  s.style = style;
  s.indent = start.col - 1;
  s.value = value;
  ScanRefsInto(&s, value, start);
  return s;
}

static inline StringNode BuildStringNodeFromToken(const Token tok) {
  return BuildStyledStringNode(TrimQuotes(tok.data), tok.range.start, tok.range.end, tok.style);
}

static inline StringNode BuildStringNode(const StrView value, const Pos start, const Pos end) {
  return BuildStyledStringNode(value, start, end, kPlainScalarStyle);
}

typedef struct {
  StrView text;
  Pos start;
  Pos end;
} SpanItem;

static inline void SplitFlowList(const StrView raw, const Pos raw_start, SpanItem** out, size_t* out_len,
                                 size_t* out_cap) {
  size_t i = 0;
  size_t n = raw.len;
  if (n >= 2 && raw.start[0] == '[' && raw.start[n - 1] == ']') {
    i = 1;
    n -= 1;
  }

  size_t start = i;
  bool in_quote = false;
  char qc = 0;
  while (i <= n) {
    const bool at_end = i == n;
    const bool is_sep = at_end || (!in_quote && raw.start[i] == ',');
    if (!is_sep && !at_end) {
      const char c = raw.start[i];
      if (in_quote) {
        if (c == qc)
          in_quote = false;
      } else if (c == '"' || c == '\'') {
        in_quote = true;
        qc = c;
      }
    }
    if (is_sep) {
      StrView piece = TrimView((StrView){.start = raw.start + start, .len = i - start});
      const StrView unquoted = TrimQuotes(piece);
      if (unquoted.len > 0) {
        const size_t rel_offset = (size_t)(unquoted.start - raw.start);
        if (*out_len + 1 > *out_cap) {
          *out_cap = *out_cap == 0 ? 4 : *out_cap * 2;
          *out = (SpanItem*)realloc(*out, sizeof(SpanItem) * (*out_cap));
        }
        (*out)[*out_len].text = unquoted;
        (*out)[*out_len].start = PositionAtOffset(raw_start, raw, rel_offset);
        (*out)[*out_len].end = PositionAtOffset(raw_start, raw, rel_offset + unquoted.len);
        (*out_len)++;
      }
      start = i + 1;
    }
    i++;
  }
}

static inline RefNode MakeTaskRef(const StrView name, const Pos start, const Pos end) {
  RefNode ref = {0};
  ref.kind = kRefKind;
  ref.ref_kind = kTaskRefKind;
  ref.name = name;
  ref.start = start;
  ref.end = end;
  return ref;
}

static bool TryReadTaskCall(Parser* p, StrView* out_name, Pos* out_start, Pos* out_end, const int row) {
  const Token peek = PeekTok(p);
  if (peek.kind != kKeyToken || !StrViewEqualsCStr(peek.data, "task") || peek.range.start.row != row)
    return false;

  NextTok(p);
  const Token value = PeekTok(p);
  if (value.range.start.row != row || (value.kind != kStringToken && value.kind != kNumberToken))
    return false;
  NextTok(p);

  *out_name = TrimQuotes(value.data);
  *out_start = value.range.start;
  *out_end = value.range.end;
  return true;
}

static inline void ParseDepsListItem(Parser* p, Frame* frame) {
  const Token dash = NextTok(p);
  const int row = dash.range.start.row;
  const Token peek = PeekTok(p);

  StrView name;
  Pos start, end;
  bool is_call = false;
  if (peek.kind == kKeyToken && StrViewEqualsCStr(peek.data, "task"))
    is_call = TryReadTaskCall(p, &name, &start, &end, row);

  if (!is_call) {
    const Token value = PeekTok(p);
    if (value.range.start.row != row || (value.kind != kStringToken && value.kind != kNumberToken)) {
      RefNode* stub = AppendNewRefInSeq(&frame->task->deps);
      *stub = MakeTaskRef((StrView){0}, dash.range.end, dash.range.end);
      stub->from = (DocumentNode*)frame->task;
      MarkNodeIncomplete((DocumentNode*)stub);
      MarkDocumentAsPartial(p->doc);
      return;
    }
    NextTok(p);
    name = TrimQuotes(value.data);
    start = value.range.start;
    end = value.range.end;
  }

  RefNode* ref = AppendNewRefInSeq(&frame->task->deps);
  *ref = MakeTaskRef(name, start, end);
  ref->from = (DocumentNode*)frame->task;
  AttachOrFlushPendingComment(p, (DocumentNode*)ref, row);
  MaybeAttachTrailingComment(p, (DocumentNode*)ref, row);
}

static inline void ParseCmdsListItem(Parser* p, Frame* frame) {
  const Token dash = NextTok(p);
  const int row = dash.range.start.row;
  const Token peek = PeekTok(p);

  CommandNode* cmd = AppendNewCommandInSeq(&frame->task->cmds);
  cmd->kind = kCommandKind;
  cmd->start = dash.range.start;

  StrView call_name;
  Pos call_start, call_end;
  if (peek.kind == kKeyToken && StrViewEqualsCStr(peek.data, "task") &&
      TryReadTaskCall(p, &call_name, &call_start, &call_end, row)) {
    cmd->task_call = MakeTaskRef(call_name, call_start, call_end);
    cmd->task_call.from = (DocumentNode*)cmd;
    cmd->end = call_end;
  } else if (peek.kind == kBlockScalarHeaderToken) {
    const Token header = NextTok(p);
    char chomp = '\0';
    if (header.data.len > 1 && (header.data.start[1] == '-' || header.data.start[1] == '+'))
      chomp = header.data.start[1];
    const Pos content_start = (Pos){.row = header.range.end.row + 1, .col = 1};
    const StrView content = LexerConsumeBlockScalar(&p->lexer, LineIndent(dash), chomp);
    const Pos content_end = (Pos){.row = p->lexer.line, .col = p->lexer.col};
    cmd->cmd = BuildStyledStringNode(content, content_start, content_end, header.style);
    cmd->start = header.range.start;
    cmd->end = content_end;
  } else if (peek.range.start.row == row && (peek.kind == kStringToken || peek.kind == kNumberToken)) {
    const Token value = NextTok(p);
    cmd->cmd = BuildStringNodeFromToken(value);
    cmd->end = value.range.end;
  } else if (peek.kind == kKeyToken && peek.range.start.row == row) {
    cmd->cmd.kind = kStringKind;
    cmd->cmd.start = peek.range.start;
    cmd->cmd.end = peek.range.start;
    cmd->end = peek.range.end;
    AttachOrFlushPendingComment(p, (DocumentNode*)cmd, row);
    PushFrame(p, (Frame){
                     .indent = LineIndent(dash),
                     .kind = kFrameCmdBody,
                     .task = frame->task,
                     .command = cmd,
                 });
    return;
  } else {
    cmd->cmd.kind = kStringKind;
    cmd->cmd.start = dash.range.end;
    cmd->cmd.end = dash.range.end;
    cmd->end = dash.range.end;
    MarkNodeIncomplete((DocumentNode*)cmd);
    MarkNodeIncomplete((DocumentNode*)&cmd->cmd);
    MarkDocumentAsPartial(p->doc);
  }

  AttachOrFlushPendingComment(p, (DocumentNode*)cmd, row);
  MaybeAttachTrailingComment(p, (DocumentNode*)cmd, row);
}

static inline void ParseInlineDeps(Parser* p, TaskNode* task, const Token value_tok) {
  SpanItem* items = NULL;
  size_t items_len = 0;
  size_t items_cap = 0;
  SplitFlowList(value_tok.data, value_tok.range.start, &items, &items_len, &items_cap);

  for (size_t i = 0; i < items_len; i++) {
    RefNode* ref = AppendNewRefInSeq(&task->deps);
    *ref = MakeTaskRef(items[i].text, items[i].start, items[i].end);
    ref->from = (DocumentNode*)task;
  }
  free(items);
  (void)p;
}

static inline void HandleVarBodyKey(Parser* p, Frame* frame, const StrView key, const Token key_tok,
                                    const Token value_tok, const bool has_value, const int indent) {
  VarNode* var = frame->var;
  if (StrViewEqualsCStr(key, "sh")) {
    var->var_kind = kShellVarNodeKind;
    var->command = NewCommandNode();
    var->command->start = key_tok.range.start;
    var->command->end = has_value ? value_tok.range.end : key_tok.range.end;
    if (has_value)
      var->command->cmd = BuildStringNodeFromToken(value_tok);

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther});
  } else if (StrViewEqualsCStr(key, "ref")) {
    var->var_kind = kRefVarNodeKind;
    StrView target = TrimQuotes(value_tok.data);
    Pos start = value_tok.range.start;
    if (target.len > 0 && target.start[0] == '.') {
      target.start++;
      target.len--;
      start.col++;
    }

    var->ref = (RefNode*)calloc(1, sizeof(RefNode));
    *var->ref = MakeTaskRef(target, start, value_tok.range.end);
    var->ref->ref_kind = kVarRefKind;
    var->ref->from = (DocumentNode*)var;
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther});
  } else if (StrViewEqualsCStr(key, "map")) {
    var->var_kind = kMapVarNodeKind;
    var->value = (DocumentNode*)NewMapNode();
    var->value->start = key_tok.range.start;
    var->value->end = key_tok.range.end;
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameMapBody, .var = var});
  } else if (StrViewEqualsCStr(key, "value")) {
    var->var_kind = kScalarVarNodeKind;
    if (has_value) {
      var->value = ClassifyScalarValue(value_tok);
      if (var->value->kind == kStringKind)
        ScanRefsInto((StringNode*)var->value, ((StringNode*)var->value)->value, var->value->start);
    }

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther});
  } else if (StrViewEqualsCStr(key, "secret")) {
    var->secret = has_value && StrViewEqualsCStr(TrimQuotes(value_tok.data), "true");
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther});
  } else {
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther});
  }
}

static inline void HandleMapEntry(VarNode* var, const StrView key, const Token key_tok, const Token value_tok,
                                  const bool has_value) {
  MapNode* map = (MapNode*)var->value;
  MapEntryNode* entry = AppendNewMapEntryInSeq(&map->entries);
  entry->kind = kMapEntryKind;
  entry->key = key;
  entry->start = key_tok.range.start;
  entry->end = has_value ? value_tok.range.end : key_tok.range.end;
  entry->key_range = key_tok.range;
  entry->indent = key_tok.indent;
  entry->leading_ws = key_tok.leading_ws;
  if (has_value)
    entry->value_range = value_tok.range;
  else
    MarkNodeIncomplete((DocumentNode*)entry);
  if (has_value)
    entry->value = ClassifyScalarValue(value_tok);
}

static inline void ParseVarOrEnvKey(Parser* p, Frame* frame, const StrView key, const Token key_tok,
                                    const Token value_tok, const bool has_value, const int indent, const int row,
                                    VarSeq* target_seq) {
  VarNode* var = NewVarNode();
  var->name = key;
  var->start = key_tok.range.start;
  var->end = key_tok.range.end;
  StampKeyedMeta(p, (DocumentNode*)var, key_tok, value_tok, has_value);
  if (has_value) {
    var->var_kind = kScalarVarNodeKind;
    var->value = ClassifyScalarValue(value_tok);
    if (var->value->kind == kStringKind)
      ScanRefsInto((StringNode*)var->value, ((StringNode*)var->value)->value, var->value->start);
  }

  AttachOrFlushPendingComment(p, (DocumentNode*)var, row);
  MaybeAttachTrailingComment(p, (DocumentNode*)var, row);

  VarNode* slot = AppendNewVarInSeq(target_seq);
  *slot = *var;
  free(var);

  if (slot->var_kind == kScalarVarNodeKind) {
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther});
  } else {
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameVarBody, .task = frame->task, .var = slot});
  }
}

static inline IncludeNode* ParseIncludeEntry(Parser* p, const StrView alias, const Token key_tok, const Token value_tok,
                                             const bool has_value) {
  IncludeNode* include = AppendNewIncludeInSeq(&p->doc->includes);
  include->kind = kIncludeKind;
  include->start = key_tok.range.start;
  include->end = has_value ? value_tok.range.end : key_tok.range.end;
  include->key_range = key_tok.range;
  include->indent = key_tok.indent;
  if (has_value) {
    include->taskfile = TrimQuotes(value_tok.data);
    include->value_range = value_tok.range;
  }

  StringNode* alias_slot = AppendNewStringInSeq(&include->aliases);
  *alias_slot = BuildStringNode(alias, key_tok.range.start, key_tok.range.end);
  return include;
}

static inline void ParseStringListItem(Parser* p, Frame* frame) {
  const Token dash = NextTok(p);
  const int row = dash.range.start.row;
  const Token value = PeekTok(p);

  if (value.range.start.row != row || (value.kind != kStringToken && value.kind != kNumberToken)) {
    StringNode* stub = AppendNewStringInSeq(frame->strings);
    stub->kind = kStringKind;
    stub->start = dash.range.end;
    stub->end = dash.range.end;
    MarkNodeIncomplete((DocumentNode*)stub);
    MarkDocumentAsPartial(p->doc);
    return;
  }

  NextTok(p);

  StringNode* slot = AppendNewStringInSeq(frame->strings);
  *slot = BuildStringNodeFromToken(value);
  AttachOrFlushPendingComment(p, (DocumentNode*)slot, row);
  MaybeAttachTrailingComment(p, (DocumentNode*)slot, row);
}

static inline void ParseStatusListItem(Parser* p, Frame* frame) {
  const Token dash = NextTok(p);
  const int row = dash.range.start.row;
  const Token value = PeekTok(p);

  CommandNode* cmd = AppendNewCommandInSeq(&frame->task->status_cmds);
  cmd->kind = kCommandKind;
  cmd->start = dash.range.start;

  if (value.range.start.row != row || (value.kind != kStringToken && value.kind != kNumberToken)) {
    cmd->cmd.kind = kStringKind;
    cmd->cmd.start = dash.range.end;
    cmd->cmd.end = dash.range.end;
    cmd->end = dash.range.end;
    MarkNodeIncomplete((DocumentNode*)cmd);
    MarkDocumentAsPartial(p->doc);
    return;
  }

  NextTok(p);

  cmd->cmd = BuildStringNodeFromToken(value);
  cmd->end = value.range.end;
  AttachOrFlushPendingComment(p, (DocumentNode*)cmd, row);
  MaybeAttachTrailingComment(p, (DocumentNode*)cmd, row);
}

static inline void ParsePreconditionListItem(Parser* p, Frame* frame) {
  const Token dash = NextTok(p);
  const int row = dash.range.start.row;

  PreconditionNode* pre = AppendNewPreconditionInSeq(&frame->task->preconditions);
  pre->kind = kPreconditionKind;
  pre->start = dash.range.start;
  pre->end = dash.range.end;

  const Token peek = PeekTok(p);
  if (peek.range.start.row == row && (peek.kind == kStringToken || peek.kind == kNumberToken)) {
    NextTok(p);
    pre->command = NewCommandNode();
    pre->command->start = peek.range.start;
    pre->command->cmd = BuildStringNodeFromToken(peek);
    pre->command->end = peek.range.end;
    pre->end = peek.range.end;
    AttachOrFlushPendingComment(p, (DocumentNode*)pre, row);
    MaybeAttachTrailingComment(p, (DocumentNode*)pre, row);
    return;
  }

  if (peek.kind == kKeyToken && peek.range.start.row == row) {
    AttachOrFlushPendingComment(p, (DocumentNode*)pre, row);
    PushFrame(p, (Frame){
                     .indent = LineIndent(dash),
                     .kind = kFramePreconditionBody,
                     .task = frame->task,
                     .precondition = pre,
                 });
    return;
  }

  MarkNodeIncomplete((DocumentNode*)pre);
  MarkDocumentAsPartial(p->doc);
}

static inline void HandlePreconditionBodyKey(Parser* p, Frame* frame, const StrView key, const Token value_tok,
                                             const bool has_value, const int indent) {
  PreconditionNode* pre = frame->precondition;

  if (StrViewEqualsCStr(key, "sh")) {
    pre->command = NewCommandNode();
    pre->command->start = value_tok.range.start;
    pre->command->end = value_tok.range.end;
    if (has_value)
      pre->command->cmd = BuildStringNodeFromToken(value_tok);

    if (has_value)
      pre->end = MaxPos(pre->end, value_tok.range.end);
  } else if (StrViewEqualsCStr(key, "msg")) {
    if (has_value) {
      pre->message = (StringNode*)calloc(1, sizeof(StringNode));
      *pre->message = BuildStringNodeFromToken(value_tok);
      pre->end = MaxPos(pre->end, value_tok.range.end);
    }
  }

  PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
}

static bool TokenIsTrue(const Token value_tok, const bool has_value);
static void AppendInlineStrings(Parser* p, StringSeq* seq, const Token value_tok);

static inline ShellOpts ParseShellOptName(const StrView name) {
  if (StrViewEqualsCStr(name, "allexport"))
    return kAllExportShellOpt;
  else if (StrViewEqualsCStr(name, "errexit"))
    return kErrExitShellOpt;
  else if (StrViewEqualsCStr(name, "noexec"))
    return kNoExecShellOpt;
  else if (StrViewEqualsCStr(name, "noglob"))
    return kNoGlobShellOpt;
  else if (StrViewEqualsCStr(name, "nounset"))
    return kNoUnsetShellOpt;
  else if (StrViewEqualsCStr(name, "xtrace"))
    return kXTraceShellOpt;
  else if (StrViewEqualsCStr(name, "pipefail"))
    return kPipeFailShellOpt;
  return kNoShellOpts;
}

static inline ShOpts ParseShOptName(const StrView name) {
  if (StrViewEqualsCStr(name, "expand_aliases"))
    return kExpandAliasesShOpt;
  else if (StrViewEqualsCStr(name, "globstar"))
    return kGlobStarShOpt;
  else if (StrViewEqualsCStr(name, "nullglob"))
    return kNullGlobShOpt;
  return kNoShOpts;
}

static inline void AccumulateShellOpts(ShellOpts** slot, const StrView raw, const Pos raw_start) {
  SpanItem* items = NULL;
  size_t items_len = 0;
  size_t items_cap = 0;
  SplitFlowList(raw, raw_start, &items, &items_len, &items_cap);

  if (!*slot)
    *slot = (ShellOpts*)calloc(1, sizeof(ShellOpts));
  for (size_t i = 0; i < items_len; i++)
    **slot = SetShellOpts(**slot, ParseShellOptName(items[i].text));
  free(items);
}

static inline void AccumulateShOpts(ShOpts** slot, const StrView raw, const Pos raw_start) {
  SpanItem* items = NULL;
  size_t items_len = 0;
  size_t items_cap = 0;
  SplitFlowList(raw, raw_start, &items, &items_len, &items_cap);

  if (!*slot)
    *slot = (ShOpts*)calloc(1, sizeof(ShOpts));

  for (size_t i = 0; i < items_len; i++)
    **slot = SetShOpts(**slot, ParseShOptName(items[i].text));

  free(items);
}

static inline ForAttribute ParseForAttribute(const StrView value) {
  if (StrViewEqualsCStr(value, "sources"))
    return kSourcesForAttribute;
  else if (StrViewEqualsCStr(value, "generates"))
    return kGeneratesForAttribute;
  return kInvalidForAttribute;
}

static inline void HandleForBodyKey(Parser* p, Frame* frame, const StrView key, const Token key_tok,
                                    const Token value_tok, const bool has_value, const int indent) {
  ForNode* node = frame->for_each;

  if (StrViewEqualsCStr(key, "var")) {
    node->for_kind = kVarForKind;
    if (has_value) {
      StrView name = TrimQuotes(value_tok.data);
      Pos start = value_tok.range.start;
      if (name.len > 0 && name.start[0] == '.') {
        name.start++;
        name.len--;
        start.col++;
      }

      node->var = MakeTaskRef(name, start, value_tok.range.end);
      node->var.ref_kind = kVarRefKind;
      node->var.from = (DocumentNode*)node;
    }

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
  } else if (StrViewEqualsCStr(key, "split")) {
    if (has_value)
      node->split = TrimQuotes(value_tok.data);

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
  } else if (StrViewEqualsCStr(key, "as")) {
    if (has_value)
      node->as = TrimQuotes(value_tok.data);

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
  } else if (StrViewEqualsCStr(key, "matrix")) {
    node->for_kind = kMatrixForKind;
    node->matrix = NewMapNode();
    node->matrix->start = key_tok.range.start;
    node->matrix->end = key_tok.range.end;
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameForMatrix, .task = frame->task, .for_each = node});
  } else {
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
  }
}

static inline void HandleCmdBodyKey(Parser* p, Frame* frame, const StrView key, const Token key_tok,
                                    const Token value_tok, const bool has_value, const int indent) {
  CommandNode* cmd = frame->command;

  cmd->end = MaxPos(cmd->end, has_value ? value_tok.range.end : key_tok.range.end);

  if (StrViewEqualsCStr(key, "cmd")) {
    if (has_value)
      cmd->cmd = BuildStringNodeFromToken(value_tok);

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
  } else if (StrViewEqualsCStr(key, "task")) {
    if (has_value) {
      cmd->task_call = MakeTaskRef(TrimQuotes(value_tok.data), value_tok.range.start, value_tok.range.end);
      cmd->task_call.from = (DocumentNode*)cmd;
    }

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
  } else if (StrViewEqualsCStr(key, "if")) {
    if (has_value) {
      cmd->if_expr = (StringNode*)calloc(1, sizeof(StringNode));
      *cmd->if_expr = BuildStringNodeFromToken(value_tok);
    }

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
  } else if (StrViewEqualsCStr(key, "silent")) {
    cmd->silent = TokenIsTrue(value_tok, has_value);
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
  } else if (StrViewEqualsCStr(key, "ignore_error")) {
    cmd->ignore_error = TokenIsTrue(value_tok, has_value);
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
  } else if (StrViewEqualsCStr(key, "platforms")) {
    if (has_value)
      AppendInlineStrings(p, &cmd->platforms, value_tok);

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameStringList, .task = frame->task, .strings = &cmd->platforms});
  } else if (StrViewEqualsCStr(key, "set")) {
    if (has_value)
      AccumulateShellOpts(&cmd->set, value_tok.data, value_tok.range.start);

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
  } else if (StrViewEqualsCStr(key, "shopt")) {
    if (has_value)
      AccumulateShOpts(&cmd->shopt, value_tok.data, value_tok.range.start);

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
  } else if (StrViewEqualsCStr(key, "for")) {
    cmd->for_each = NewForNode();
    cmd->for_each->start = key_tok.range.start;
    cmd->for_each->end = has_value ? value_tok.range.end : key_tok.range.end;
    cmd->for_each->key_range = key_tok.range;
    if (has_value) {
      const StrView value = TrimQuotes(value_tok.data);
      const ForAttribute attr = ParseForAttribute(value);
      if (attr != kInvalidForAttribute) {
        cmd->for_each->for_kind = kAttributeForKind;
        cmd->for_each->attribute = attr;
      } else {
        cmd->for_each->for_kind = kListForKind;
        AppendInlineStrings(p, &cmd->for_each->items, value_tok);
      }

      PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
    } else {
      PushFrame(p, (Frame){.indent = indent, .kind = kFrameForBody, .task = frame->task, .for_each = cmd->for_each});
    }
  } else if (StrViewEqualsCStr(key, "defer")) {
    cmd->defer = NewDeferNode();
    cmd->defer->start = key_tok.range.start;
    cmd->defer->end = has_value ? value_tok.range.end : key_tok.range.end;
    cmd->defer->key_range = key_tok.range;
    if (has_value) {
      cmd->defer->cmd = (StringNode*)calloc(1, sizeof(StringNode));
      *cmd->defer->cmd = BuildStringNodeFromToken(value_tok);
      PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
    } else {
      PushFrame(p, (Frame){.indent = indent, .kind = kFrameCmdBody, .task = frame->task, .command = cmd});
    }
  } else {
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
  }
}

static inline void HandleOutputBodyKey(Parser* p, Frame* frame, const StrView key, const Token value_tok,
                                       const bool has_value, const int indent) {
  OutputNode* out = p->doc->output;
  if (!out) {
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther});
    return;
  }

  if (StrViewEqualsCStr(key, "group")) {
    out->output_kind = kGroupOutputKind;
  } else if (StrViewEqualsCStr(key, "begin")) {
    if (has_value)
      out->begin = TrimQuotes(value_tok.data);
  } else if (StrViewEqualsCStr(key, "end")) {
    if (has_value)
      out->end_template = TrimQuotes(value_tok.data);
  } else if (StrViewEqualsCStr(key, "error_only")) {
    out->error_only = TokenIsTrue(value_tok, has_value);
  }

  PushFrame(p, (Frame){.indent = indent, .kind = kFrameOutput});
}

static inline void HandleIncludeBodyKey(Parser* p, Frame* frame, const StrView key, const Token value_tok,
                                        const bool has_value, const int indent) {
  IncludeNode* include = frame->include;
  if (StrViewEqualsCStr(key, "taskfile")) {
    if (has_value)
      include->taskfile = TrimQuotes(value_tok.data);
  } else if (StrViewEqualsCStr(key, "dir")) {
    if (has_value)
      include->dir = TrimQuotes(value_tok.data);
  } else if (StrViewEqualsCStr(key, "checksum")) {
    if (has_value)
      include->checksum = TrimQuotes(value_tok.data);
  } else if (StrViewEqualsCStr(key, "optional")) {
    include->optional = TokenIsTrue(value_tok, has_value);
  } else if (StrViewEqualsCStr(key, "flatten")) {
    include->flatten = TokenIsTrue(value_tok, has_value);
  } else if (StrViewEqualsCStr(key, "internal")) {
    include->internal = TokenIsTrue(value_tok, has_value);
  } else if (StrViewEqualsCStr(key, "aliases")) {
    if (has_value)
      AppendInlineStrings(p, &include->aliases, value_tok);

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameStringList, .strings = &include->aliases});
    return;
  } else if (StrViewEqualsCStr(key, "excludes")) {
    if (has_value)
      AppendInlineStrings(p, &include->excludes, value_tok);

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameStringList, .strings = &include->excludes});
    return;
  } else if (StrViewEqualsCStr(key, "vars")) {
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameVars, .include = include});
    return;
  }

  PushFrame(p, (Frame){.indent = indent, .kind = kFrameIncludeBody, .include = include});
}

static inline bool TokenIsTrue(const Token value_tok, const bool has_value) {
  return has_value && StrViewEqualsCStr(TrimQuotes(value_tok.data), "true");
}

static inline void AppendInlineStrings(Parser* p, StringSeq* seq, const Token value_tok) {
  SpanItem* items = NULL;
  size_t items_len = 0;
  size_t items_cap = 0;
  SplitFlowList(value_tok.data, value_tok.range.start, &items, &items_len, &items_cap);

  for (size_t i = 0; i < items_len; i++) {
    StringNode* slot = AppendNewStringInSeq(seq);
    *slot = BuildStringNode(items[i].text, items[i].start, items[i].end);
  }

  free(items);
  (void)p;
}

static inline bool TryParseTaskStringSeqKey(Parser* p, TaskNode* task, const StrView key, const Token value_tok,
                                            const bool has_value, const int indent) {
  StringSeq* target = NULL;
  if (StrViewEqualsCStr(key, "aliases"))
    target = &task->aliases;
  else if (StrViewEqualsCStr(key, "prompt"))
    target = &task->prompts;
  else if (StrViewEqualsCStr(key, "sources"))
    target = &task->sources;
  else if (StrViewEqualsCStr(key, "generates"))
    target = &task->generates;
  else if (StrViewEqualsCStr(key, "platforms"))
    target = &task->platforms;
  else if (StrViewEqualsCStr(key, "dotenv"))
    target = &task->dotenvs;

  if (!target)
    return false;

  if (has_value)
    AppendInlineStrings(p, target, value_tok);

  PushFrame(p, (Frame){.indent = indent, .kind = kFrameStringList, .task = task, .strings = target});
  return true;
}

static inline bool TryParseTaskScalarKey(TaskNode* task, const StrView key, const Token value_tok,
                                         const bool has_value) {
  if (!has_value)
    return false;

  const StrView value = TrimQuotes(value_tok.data);
  if (StrViewEqualsCStr(key, "desc"))
    task->desc = value;
  else if (StrViewEqualsCStr(key, "summary"))
    task->summary = value;
  else if (StrViewEqualsCStr(key, "dir"))
    task->dir = value;
  else if (StrViewEqualsCStr(key, "label"))
    task->label = value;
  else if (StrViewEqualsCStr(key, "prefix"))
    task->prefix = value;
  else if (StrViewEqualsCStr(key, "interval"))
    task->interval = value;
  else
    return false;

  return true;
}

static inline bool TryParseTaskFlagKey(TaskNode* task, const StrView key, const Token value_tok, const bool has_value) {
  TaskFlags flag = kNoTaskFlags;
  if (StrViewEqualsCStr(key, "silent"))
    flag = kSilentFlag;
  else if (StrViewEqualsCStr(key, "internal"))
    flag = kInternalFlag;
  else if (StrViewEqualsCStr(key, "interactive"))
    flag = kInteractiveFlag;
  else if (StrViewEqualsCStr(key, "prefixed"))
    flag = kPrefixedFlag;
  else if (StrViewEqualsCStr(key, "ignore_error"))
    flag = kIgnoreErrorFlag;
  else if (StrViewEqualsCStr(key, "watch"))
    flag = kWatchFlag;
  else
    return false;

  if (TokenIsTrue(value_tok, has_value))
    SetTaskFlags(task, task->flags | flag);
  return true;
}

static inline bool TryParseTaskEnumKey(TaskNode* task, const StrView key, const Token value_tok, const bool has_value) {
  if (!has_value)
    return false;

  const StrView value = TrimQuotes(value_tok.data);
  if (StrViewEqualsCStr(key, "method")) {
    if (StrViewEqualsCStr(value, "none"))
      task->method = kNoneMethodKind;
    else if (StrViewEqualsCStr(value, "checksum"))
      task->method = kChecksumMethodKind;
    else if (StrViewEqualsCStr(value, "timestamp"))
      task->method = kTimestampMethodKind;

    return true;
  }

  if (StrViewEqualsCStr(key, "run")) {
    if (StrViewEqualsCStr(value, "always"))
      task->mode = kTaskRunAlwaysMode;
    else if (StrViewEqualsCStr(value, "once"))
      task->mode = kTaskRunOnceMode;
    else if (StrViewEqualsCStr(value, "when_changed"))
      task->mode = kTaskRunWhenChangedMode;

    return true;
  }

  return false;
}

static inline void ParseTaskBodyKey(Parser* p, Frame* frame, const StrView key, const Token value_tok,
                                    const bool has_value, const int indent) {
  TaskNode* task = frame->task;
  if (TryParseTaskScalarKey(task, key, value_tok, has_value) || TryParseTaskFlagKey(task, key, value_tok, has_value) ||
      TryParseTaskEnumKey(task, key, value_tok, has_value)) {
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = task});
    return;
  }

  if (TryParseTaskStringSeqKey(p, task, key, value_tok, has_value, indent))
    return;

  if (StrViewEqualsCStr(key, "deps")) {
    if (has_value)
      ParseInlineDeps(p, task, value_tok);

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameDepsList, .task = task});
  } else if (StrViewEqualsCStr(key, "cmds")) {
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameCmdsList, .task = task});
  } else if (StrViewEqualsCStr(key, "cmd")) {
    if (has_value) {
      task->is_short_form = true;
      CommandNode* cmd = AppendNewCommandInSeq(&task->cmds);
      cmd->kind = kCommandKind;
      cmd->start = value_tok.range.start;
      cmd->cmd = BuildStringNodeFromToken(value_tok);
      cmd->end = value_tok.range.end;
    }

    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = task});
  } else if (StrViewEqualsCStr(key, "vars")) {
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameVars, .task = task});
  } else if (StrViewEqualsCStr(key, "env")) {
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameEnv, .task = task});
  } else if (StrViewEqualsCStr(key, "status")) {
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameStatusList, .task = task});
  } else if (StrViewEqualsCStr(key, "preconditions")) {
    PushFrame(p, (Frame){.indent = indent, .kind = kFramePreconditionList, .task = task});
  } else if (StrViewEqualsCStr(key, "requires")) {
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameRequires, .task = task});
  } else {
    PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = task});
  }
}

static inline void ParseLine(Parser* p) {
  const Token tok = PeekTok(p);
  const int indent = LineIndent(tok);
  const int row = tok.range.start.row;
  PopFramesTo(p, indent);
  Frame* frame = TopFrame(p);

  if (tok.kind == kDocumentSeparatorToken) {
    NextTok(p);
    return;
  }

  if (tok.kind == kBlockCommentToken) {
    NextTok(p);
    QueuePendingComment(p, tok);
    return;
  }

  if (tok.kind == kInvalidToken) {
    NextTok(p);
    MarkDocumentAsPartial(p->doc);
    return;
  }

  if (tok.kind == kDashToken) {
    if (frame->kind == kFrameDepsList && frame->task) {
      ParseDepsListItem(p, frame);
    } else if (frame->kind == kFrameCmdsList && frame->task) {
      ParseCmdsListItem(p, frame);
    } else if (frame->kind == kFrameStringList && frame->strings) {
      ParseStringListItem(p, frame);
    } else if (frame->kind == kFrameStatusList && frame->task) {
      ParseStatusListItem(p, frame);
    } else if (frame->kind == kFramePreconditionList && frame->task) {
      ParsePreconditionListItem(p, frame);
    } else {
      NextTok(p);
      MarkDocumentAsPartial(p->doc);
    }

    return;
  }

  if (tok.kind != kKeyToken) {
    NextTok(p);
    MarkDocumentAsPartial(p->doc);
    return;
  }

  const Token key_tok = NextTok(p);
  const StrView key = key_tok.data;

  Token value_tok = {0};
  bool has_value = false;
  const Token maybe_value = PeekTok(p);
  if (maybe_value.range.start.row == row && (maybe_value.kind == kStringToken || maybe_value.kind == kNumberToken)) {
    value_tok = NextTok(p);
    has_value = true;
  } else if (maybe_value.range.start.row == row && maybe_value.kind == kBlockScalarHeaderToken) {
    const Token header = NextTok(p);
    char chomp = '\0';
    if (header.data.len > 1 && (header.data.start[1] == '-' || header.data.start[1] == '+'))
      chomp = header.data.start[1];

    const Pos content_start = (Pos){.row = header.range.end.row + 1, .col = 1};
    const StrView content = LexerConsumeBlockScalar(&p->lexer, indent, chomp);
    value_tok.data = content;
    value_tok.range = (Range){.start = content_start, .end = (Pos){.row = p->lexer.line, .col = p->lexer.col}};
    has_value = !StrViewIsEmpty(content);
  }

  switch (frame->kind) {
    case kFrameRoot:
      if (StrViewEqualsCStr(key, "tasks")) {
        PushFrame(p, (Frame){.indent = indent, .kind = kFrameTasks});
      } else if (StrViewEqualsCStr(key, "includes")) {
        PushFrame(p, (Frame){.indent = indent, .kind = kFrameIncludes});
      } else if (StrViewEqualsCStr(key, "vars")) {
        PushFrame(p, (Frame){.indent = indent, .kind = kFrameVars});
      } else if (StrViewEqualsCStr(key, "env")) {
        PushFrame(p, (Frame){.indent = indent, .kind = kFrameEnv});
      } else if (StrViewEqualsCStr(key, "version")) {
        if (has_value)
          p->doc->version = TrimQuotes(value_tok.data);

        PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther});
      } else if (StrViewEqualsCStr(key, "output")) {
        p->doc->output = NewOutputNode();
        p->doc->output->start = key_tok.range.start;
        p->doc->output->end = has_value ? value_tok.range.end : key_tok.range.end;
        p->doc->output->key_range = key_tok.range;
        if (has_value) {
          const StrView value = TrimQuotes(value_tok.data);
          p->doc->output->value_range = value_tok.range;
          if (StrViewEqualsCStr(value, "group"))
            p->doc->output->output_kind = kGroupOutputKind;
          else if (StrViewEqualsCStr(value, "prefixed"))
            p->doc->output->output_kind = kPrefixedOutputKind;
          else
            p->doc->output->output_kind = kInterleavedOutputKind;

          PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther});
        } else {
          PushFrame(p, (Frame){.indent = indent, .kind = kFrameOutput});
        }
      } else if (StrViewEqualsCStr(key, "dotenv")) {
        if (has_value)
          AppendInlineStrings(p, &p->doc->dotenv, value_tok);

        PushFrame(p, (Frame){.indent = indent, .kind = kFrameStringList, .strings = &p->doc->dotenv});
      } else {
        PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther});
      }

      break;

    case kFrameTasks: {
      TaskNode* existing = FindDocumentTask(p->doc, key);
      if (existing) {
        MarkDocumentAsPartial(p->doc);
        PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther});
        break;
      }

      TaskNode* task = NewTaskNode();
      task->name = key;
      task->start = key_tok.range.start;
      task->end = key_tok.range.end;
      StampKeyedMeta(p, (DocumentNode*)task, key_tok, value_tok, has_value);
      AttachOrFlushPendingComment(p, (DocumentNode*)task, row);
      MaybeAttachTrailingComment(p, (DocumentNode*)task, row);
      TaskNode* slot = AppendNewTaskInSeq(&p->doc->tasks);
      *slot = *task;
      free(task);

      if (has_value) {
        slot->is_short_form = true;
        CommandNode* cmd = AppendNewCommandInSeq(&slot->cmds);
        cmd->kind = kCommandKind;
        cmd->start = value_tok.range.start;
        cmd->cmd = BuildStringNodeFromToken(value_tok);
        cmd->end = value_tok.range.end;
        PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther});
      } else if (PeekTok(p).kind == kDashToken) {
        slot->is_short_form = true;
        PushFrame(p, (Frame){.indent = indent, .kind = kFrameCmdsList, .task = slot});
      } else {
        PushFrame(p, (Frame){.indent = indent, .kind = kFrameTaskBody, .task = slot});
      }

      break;
    }

    case kFrameTaskBody:
      ParseTaskBodyKey(p, frame, key, value_tok, has_value, indent);
      break;

    case kFrameVars: {
      VarSeq* target_seq = frame->include ? &frame->include->vars : frame->task ? &frame->task->vars : &p->doc->vars;
      ParseVarOrEnvKey(p, frame, key, key_tok, value_tok, has_value, indent, row, target_seq);
      break;
    }

    case kFrameEnv: {
      VarSeq* target_seq = frame->task ? &frame->task->env : &p->doc->env;
      ParseVarOrEnvKey(p, frame, key, key_tok, value_tok, has_value, indent, row, target_seq);
      break;
    }

    case kFrameVarBody:
      HandleVarBodyKey(p, frame, key, key_tok, value_tok, has_value, indent);
      break;

    case kFramePreconditionBody:
      HandlePreconditionBodyKey(p, frame, key, value_tok, has_value, indent);
      break;

    case kFrameCmdBody:
      HandleCmdBodyKey(p, frame, key, key_tok, value_tok, has_value, indent);
      break;

    case kFrameForBody:
      HandleForBodyKey(p, frame, key, key_tok, value_tok, has_value, indent);
      break;

    case kFrameForMatrix:
      if (frame->for_each && frame->for_each->matrix) {
        MapEntryNode* entry = AppendNewMapEntryInSeq(&frame->for_each->matrix->entries);
        entry->kind = kMapEntryKind;
        entry->key = key;
        entry->start = key_tok.range.start;
        entry->end = has_value ? value_tok.range.end : key_tok.range.end;
        entry->key_range = key_tok.range;
        if (has_value)
          entry->value = ClassifyScalarValue(value_tok);
      }

      PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
      break;

    case kFrameOutput:
      HandleOutputBodyKey(p, frame, key, value_tok, has_value, indent);
      break;

    case kFrameIncludeBody:
      HandleIncludeBodyKey(p, frame, key, value_tok, has_value, indent);
      break;

    case kFrameRequires:
      if (StrViewEqualsCStr(key, "vars")) {
        if (has_value)
          AppendInlineStrings(p, &frame->task->requires_vars, value_tok);

        PushFrame(p, (Frame){
                         .indent = indent,
                         .kind = kFrameStringList,
                         .task = frame->task,
                         .strings = &frame->task->requires_vars,
                     });
      } else {
        PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
      }

      break;

    case kFrameMapBody:
      HandleMapEntry(frame->var, key, key_tok, value_tok, has_value);
      PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther});
      break;

    case kFrameIncludes: {
      IncludeNode* include = ParseIncludeEntry(p, key, key_tok, value_tok, has_value);
      if (has_value) {
        PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther});
      } else {
        PushFrame(p, (Frame){.indent = indent, .kind = kFrameIncludeBody, .include = include});
      }

      break;
    }

    default:
      PushFrame(p, (Frame){.indent = indent, .kind = kFrameOther, .task = frame->task});
      break;
  }
}

static inline void ResolveRefAgainstScope(RefNode* ref, Document* doc, TaskNode* scope) {
  if (!ref || ref->to || StrViewIsEmpty(ref->name))
    return;

  if (ref->ref_kind == kTaskRefKind) {
    ref->to = (DocumentNode*)FindDocumentTask(doc, ref->name);
    return;
  }

  if (ref->ref_kind == kEnvRefKind) {
    VarNode* env_target = scope ? FindTaskEnvVar(scope, ref->name) : NULL;
    if (!env_target)
      env_target = FindDocumentEnvVar(doc, ref->name);

    ref->to = (DocumentNode*)env_target;
    return;
  }

  if (memchr(ref->name.start, ':', ref->name.len))
    return;

  VarNode* target = scope ? FindTaskVar(scope, ref->name) : NULL;
  if (!target)
    target = FindDocumentVar(doc, ref->name);
  ref->to = (DocumentNode*)target;
}

static inline void ResolveStringRefs(StringNode* str, Document* doc, TaskNode* scope) {
  if (!str)
    return;

  for (uint64_t i = 0; i < GetNumberOfRefsInString(str); i++)
    ResolveRefAgainstScope(GetStringRefAt(str, i), doc, scope);
}

static inline void ResolveVarValue(VarNode* var, Document* doc, TaskNode* scope) {
  switch (var->var_kind) {
    case kScalarVarNodeKind:
      if (var->value && IsStringNode(var->value))
        ResolveStringRefs((StringNode*)var->value, doc, scope);

      break;

    case kShellVarNodeKind:
      if (var->command)
        ResolveStringRefs(&var->command->cmd, doc, scope);

      break;

    case kRefVarNodeKind:
      ResolveRefAgainstScope(var->ref, doc, scope);
      break;

    case kMapVarNodeKind: {
      if (!var->value)
        break;

      MapNode* map = (MapNode*)var->value;
      for (uint64_t i = 0; i < GetNumberOfMapEntrysInSeq(&map->entries); i++) {
        MapEntryNode* entry = GetMapEntryInSeqAt(&map->entries, i);
        if (entry->value && IsStringNode(entry->value))
          ResolveStringRefs((StringNode*)entry->value, doc, scope);
      }

      break;
    }

    default:
      break;
  }
}

static inline void ResolveDocumentReferences(Document* doc) {
  for (uint64_t i = 0; i < GetNumberOfVarsInDocument(doc); i++)
    ResolveVarValue(GetDocumentVarAt(doc, i), doc, NULL);

  for (uint64_t i = 0; i < GetNumberOfEnvVarsInDocument(doc); i++)
    ResolveVarValue(GetDocumentEnvVarAt(doc, i), doc, NULL);

  for (uint64_t i = 0; i < GetNumberOfTasksInDocument(doc); i++) {
    TaskNode* task = GetDocumentTaskAt(doc, i);

    for (uint64_t j = 0; j < GetNumberOfRefsInSeq(&task->deps); j++)
      ResolveRefAgainstScope(GetRefInSeqAt(&task->deps, j), doc, task);

    for (uint64_t j = 0; j < GetNumberOfCommandsInSeq(&task->cmds); j++) {
      CommandNode* cmd = GetCommandInSeqAt(&task->cmds, j);
      if (IsCommandTaskCall(cmd))
        ResolveRefAgainstScope(&cmd->task_call, doc, task);
      else
        ResolveStringRefs(&cmd->cmd, doc, task);
    }

    for (uint64_t j = 0; j < GetNumberOfVarsInSeq(&task->vars); j++)
      ResolveVarValue(GetVarInSeqAt(&task->vars, j), doc, task);

    for (uint64_t j = 0; j < GetNumberOfVarsInSeq(&task->env); j++)
      ResolveVarValue(GetVarInSeqAt(&task->env, j), doc, task);
  }
}

static inline void ExtendEnd(DocumentNode* parent, DocumentNode* child) {
  if (!parent || !child)
    return;

  parent->end = MaxPos(parent->end, child->end);
}

static inline void ExtendVarRange(VarNode* var) {
  switch (var->var_kind) {
    case kScalarVarNodeKind:
      ExtendEnd((DocumentNode*)var, var->value);
      break;

    case kShellVarNodeKind:
      ExtendEnd((DocumentNode*)var, (DocumentNode*)var->command);
      break;

    case kRefVarNodeKind:
      ExtendEnd((DocumentNode*)var, (DocumentNode*)var->ref);
      break;

    case kMapVarNodeKind: {
      if (!var->value)
        break;

      MapNode* map = (MapNode*)var->value;
      for (uint64_t i = 0; i < GetNumberOfMapEntrysInSeq(&map->entries); i++) {
        MapEntryNode* entry = GetMapEntryInSeqAt(&map->entries, i);
        ExtendEnd((DocumentNode*)map, (DocumentNode*)entry);
      }

      ExtendEnd((DocumentNode*)var, var->value);
      break;
    }

    default:
      break;
  }
}

static inline void ExtendContainerRanges(Document* doc) {
  for (uint64_t i = 0; i < GetNumberOfVarsInDocument(doc); i++)
    ExtendVarRange(GetDocumentVarAt(doc, i));

  for (uint64_t i = 0; i < GetNumberOfEnvVarsInDocument(doc); i++)
    ExtendVarRange(GetDocumentEnvVarAt(doc, i));

  for (uint64_t i = 0; i < GetNumberOfIncludesInDocument(doc); i++) {
    IncludeNode* include = GetDocumentIncludeAt(doc, i);
    for (uint64_t j = 0; j < GetNumberOfIncludeAliases(include); j++)
      ExtendEnd((DocumentNode*)include, (DocumentNode*)GetIncludeAliasAt(include, j));

    for (uint64_t j = 0; j < GetNumberOfIncludeExcludes(include); j++)
      ExtendEnd((DocumentNode*)include, (DocumentNode*)GetIncludeExcludeAt(include, j));

    for (uint64_t j = 0; j < GetNumberOfVarsInSeq(&include->vars); j++) {
      VarNode* var = GetVarInSeqAt(&include->vars, j);
      ExtendVarRange(var);
      ExtendEnd((DocumentNode*)include, (DocumentNode*)var);
    }
  }

  for (uint64_t i = 0; i < GetNumberOfTasksInDocument(doc); i++) {
    TaskNode* task = GetDocumentTaskAt(doc, i);

    for (uint64_t j = 0; j < GetNumberOfCommandsInSeq(&task->cmds); j++)
      ExtendEnd((DocumentNode*)task, (DocumentNode*)GetCommandInSeqAt(&task->cmds, j));

    for (uint64_t j = 0; j < GetNumberOfRefsInSeq(&task->deps); j++)
      ExtendEnd((DocumentNode*)task, (DocumentNode*)GetRefInSeqAt(&task->deps, j));

    for (uint64_t j = 0; j < GetNumberOfVarsInSeq(&task->vars); j++) {
      VarNode* var = GetVarInSeqAt(&task->vars, j);
      ExtendVarRange(var);
      ExtendEnd((DocumentNode*)task, (DocumentNode*)var);
    }

    for (uint64_t j = 0; j < GetNumberOfVarsInSeq(&task->env); j++) {
      VarNode* var = GetVarInSeqAt(&task->env, j);
      ExtendVarRange(var);
      ExtendEnd((DocumentNode*)task, (DocumentNode*)var);
    }

    for (uint64_t j = 0; j < GetNumberOfCommandsInSeq(&task->cmds); j++) {
      CommandNode* cmd = GetCommandInSeqAt(&task->cmds, j);
      ExtendEnd((DocumentNode*)cmd, (DocumentNode*)cmd->if_expr);
      if (cmd->for_each) {
        ForNode* loop = cmd->for_each;
        for (uint64_t k = 0; k < GetNumberOfItemsInFor(loop); k++)
          ExtendEnd((DocumentNode*)loop, (DocumentNode*)GetForItemAt(loop, k));

        if (loop->for_kind == kVarForKind)
          ExtendEnd((DocumentNode*)loop, (DocumentNode*)&loop->var);

        if (loop->matrix) {
          for (uint64_t k = 0; k < GetNumberOfMapEntrysInSeq(&loop->matrix->entries); k++)
            ExtendEnd((DocumentNode*)loop->matrix, (DocumentNode*)GetMapEntryInSeqAt(&loop->matrix->entries, k));

          ExtendEnd((DocumentNode*)loop, (DocumentNode*)loop->matrix);
        }

        ExtendEnd((DocumentNode*)cmd, (DocumentNode*)loop);
      }

      if (cmd->defer) {
        ExtendEnd((DocumentNode*)cmd->defer, (DocumentNode*)cmd->defer->cmd);
        ExtendEnd((DocumentNode*)cmd, (DocumentNode*)cmd->defer);
      }

      for (uint64_t k = 0; k < GetNumberOfStringsInSeq(&cmd->platforms); k++)
        ExtendEnd((DocumentNode*)cmd, (DocumentNode*)GetStringInSeqAt(&cmd->platforms, k));

      ExtendEnd((DocumentNode*)task, (DocumentNode*)cmd);
    }

    for (uint64_t j = 0; j < GetNumberOfCommandsInSeq(&task->status_cmds); j++)
      ExtendEnd((DocumentNode*)task, (DocumentNode*)GetCommandInSeqAt(&task->status_cmds, j));

    for (uint64_t j = 0; j < GetNumberOfPreconditionsInSeq(&task->preconditions); j++) {
      PreconditionNode* pre = GetPreconditionInSeqAt(&task->preconditions, j);
      ExtendEnd((DocumentNode*)pre, (DocumentNode*)pre->command);
      ExtendEnd((DocumentNode*)pre, (DocumentNode*)pre->message);
      ExtendEnd((DocumentNode*)task, (DocumentNode*)pre);
    }

#define EXTEND_TASK_STRING_SEQ(Singular, Plural, Field)                \
  for (uint64_t j = 0; j < GetNumberOfStringsInSeq(&task->Field); j++) \
    ExtendEnd((DocumentNode*)task, (DocumentNode*)GetStringInSeqAt(&task->Field, j));
    FOR_EACH_TASK_STRING_SEQ(EXTEND_TASK_STRING_SEQ)
#undef EXTEND_TASK_STRING_SEQ

    for (uint64_t j = 0; j < GetNumberOfStringsInSeq(&task->dotenvs); j++)
      ExtendEnd((DocumentNode*)task, (DocumentNode*)GetStringInSeqAt(&task->dotenvs, j));
  }
}

TaskfileParseResult ParseTaskfileDocumentStr(const char* data, const size_t data_len) {
  if (!data || data_len == 0)
    PARSE_ERROR("document is empty");

  Parser p = {0};
  InitLexer(&p.lexer, data);
  p.doc = NewDocument(NULL);
  PushFrame(&p, (Frame){.indent = -1, .kind = kFrameRoot});

  for (;;) {
    const Token peek = PeekTok(&p);
    if (peek.kind == kEofToken)
      break;
    ParseLine(&p);
  }

  FlushPendingCommentAsFloating(&p);
  free(p.stack);

  p.doc->newline_style = p.lexer.newline_style;
  p.doc->indent_width = LexerIndentWidth(&p.lexer);
  p.doc->indent_tabs = p.lexer.saw_tab_indent;
  p.doc->blank_lines = p.lexer.blank_lines;

  ResolveDocumentReferences(p.doc);
  ExtendContainerRanges(p.doc);

  return (TaskfileParseResult){.success = true, .doc = p.doc};
}
