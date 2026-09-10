#ifndef TASKFILE_PARSER_DOC_H
#define TASKFILE_PARSER_DOC_H

#include "taskfile_parser.h"

struct _Document {
  DEFINE_DOCUMENT_NODE_FIELDS;

  char* path;

  StringNode* dotenv;
  size_t dotenv_len;
  size_t dotenv_cap;

  IncludeNode* includes;
  size_t includes_len;
  size_t includes_cap;

  CommentNode* comments;
  size_t comments_len;
  size_t comments_cap;

  TaskNode* tasks;
  size_t tasks_len;
  size_t tasks_cap;

  bool fragment;
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

#endif  // TASKFILE_PARSER_DOC_H
