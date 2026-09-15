#ifndef TASKFILE_PARSER_DOC_H
#define TASKFILE_PARSER_DOC_H

#include "taskfile_parser.h"

struct _Document {
  DEFINE_DOCUMENT_NODE_FIELDS;

  char* path;
  StrView version;

  bool fragment;
  bool partial;

  NewlineStyle newline_style;
  int indent_width;
  bool indent_tabs;
  LineSeq blank_lines;
  OutputNode* output;

  ShellOpts* set;
  ShOpts* shopt;
  StringSeq dotenv;
  IncludeSeq includes;
  TaskSeq tasks;
  VarSeq vars;
  VarSeq env;
};

Document* NewDocument(const char* path);

static inline void SetDocumentFragment(Document* lhs, const bool rhs) {
  if (!lhs)
    return;

  lhs->fragment = rhs;
}

static inline void MarkDocumentAsFragment(Document* rhs) {
  return SetDocumentFragment(rhs, true);
}

static inline void ResetDocumentFragmentMarker(Document* rhs) {
  return SetDocumentFragment(rhs, false);
}

static inline void SetDocumentPartial(Document* lhs, const bool rhs) {
  if (!lhs)
    return;

  lhs->partial = rhs;
}

static inline void MarkDocumentAsPartial(Document* rhs) {
  return SetDocumentPartial(rhs, true);
}

TaskNode* NewTaskNode(void);
VarNode* NewVarNode(void);
CommandNode* NewCommandNode(void);
MapNode* NewMapNode(void);
CommentNode* NewCommentNode(void);
ForNode* NewForNode(void);
DeferNode* NewDeferNode(void);
OutputNode* NewOutputNode(void);

void AddCommentToNode(DocumentNode* node, CommentNode comment);

#endif  // TASKFILE_PARSER_DOC_H
