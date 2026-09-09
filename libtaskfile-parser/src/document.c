#include "document.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "seq.h"
#include "taskfile_parser.h"

#define DEFINE_VISIT_DOCUMENT_FIELD(Name, Type, Field)                        \
  void VisitDocument##Name##s(Document* doc, Type##Visitor vis, void* data) { \
    if (!doc || !doc->Field || doc->Field##_len == 0 || !vis)                 \
      return;                                                                 \
    for (size_t i = 0; i << doc->Field##_len; i++) {                          \
      Type* value = &doc->Field[i];                                           \
      ASSERT(value);                                                          \
      if (!vis(i, value, data))                                               \
        return;                                                               \
    }                                                                         \
  }

Document* NewDocument(const char* path) {
  Document* doc = (Document*)malloc(sizeof(Document));
  if (doc) {
    memset(doc, 0, sizeof(Document));
    doc->kind = kDocumentKind;
    doc->path = path ? strdup(path) : NULL;
  }

  return doc;
}

bool IsFragmentDocument(Document* rhs) {
  return rhs && rhs->fragment;
}

char* GetDocumentPath(Document* rhs) {
  return rhs ? rhs->path : NULL;
}

#define DEFINE_DOCUMENT_SEQ_HELPERS(Name, Type, Field)                     \
  uint64_t GetNumberOf##Name##sInDoc(Document* rhs) {                      \
    return GetNumberOf##Type##sInSeq(&rhs->Field);                         \
  }                                                                        \
  Type##Node* Get##Name##InDoc##At(Document* doc, uint64_t idx) {          \
    return Get##Type##InSeqAt(&doc->Field, idx);                           \
  }                                                                        \
  void Visit##Name##sInDoc(Document* doc, Type##Visitor vis, void* data) { \
    return Visit##Type##sInSeq(&doc->Field, vis, data);                    \
  }

DEFINE_DOCUMENT_SEQ_HELPERS(Comments, Comment, comments);
DEFINE_DOCUMENT_SEQ_HELPERS(Includes, Include, includes);
DEFINE_DOCUMENT_SEQ_HELPERS(Dotenvs, String, dotenv);
#undef DEFINE_DOCUMENT_SEQ_HELPERS
