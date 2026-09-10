#ifndef TASKFILE_PARSER_DOC_H
#define TASKFILE_PARSER_DOC_H

#include "taskfile_parser.h"

struct _Document {
  DEFINE_DOCUMENT_NODE_FIELDS;

  char* path;

  bool fragment;

  ShellOpts* set;
  ShOpts* shopt;
  StringSeq dotenv;
  IncludeSeq includes;
  CommentSeq comments;
  TaskSeq tasks;
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
