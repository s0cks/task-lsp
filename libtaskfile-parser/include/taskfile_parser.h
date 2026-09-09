#ifndef TASKFILE_PARSER_H
#define TASKFILE_PARSER_H

// NOLINTBEGIN(modernize-use-using,modernize-use-trailing-return-type,cppcoreguidelines-pro-type-cstyle-cast)
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

typedef struct _DocumentNode DocumentNode;

typedef struct {
  int row;
  int col;
} Position;

typedef struct {
  char* start;
  size_t len;
} str_view;

#define FOR_EACH_TOKEN_KIND(V)

// clang-format off
typedef enum {
  kInvalidToken = 0,
#define DEFINE_KIND(Name) k##Name##Token,
  FOR_EACH_TOKEN_KIND(DEFINE_KIND)
#undef DEFINE_KIND
  kTotalNumberOfTokenKinds,
} TokenKind;
// clang-format on

typedef struct {
  TokenKind kind;
  str_view data;
  Position start;
  Position end;
} Token;

#define FOR_EACH_DIAGNOSTIC_LEVEL(V) \
  V(Info)                            \
  V(Warning)                         \
  V(Error)                           \
  V(Debug)

// clang-format off
typedef enum {
#define DEFINE_LEVEL(Name) k##Name##Level,
  FOR_EACH_DIAGNOSTIC_LEVEL(DEFINE_LEVEL)
#undef DEFINE_LEVEL
  kTotalNumberOfDiagnosticLevels,
} DiagnosticLevel;
// clang-format on

typedef struct {
  DocumentNode* owner;
  DiagnosticLevel level;
  Position start;
  Position end;
  char* message;
} Diagnostic;

#define FOR_EACH_DOCUMENT_NODE_KIND(V) \
  V(Document)                          \
  V(Ref)                               \
  V(Command)                           \
  V(Var)                               \
  V(Task)                              \
  V(Comment)

// clang-format off
typedef enum {
  kInvalidDocumentNodeKind = 0,
#define DEFINE_KIND(Name) k##Name##Kind,
  FOR_EACH_DOCUMENT_NODE_KIND(DEFINE_KIND)
#undef DEFINE_KIND
  kTotalNumberOfDocumentNodeKinds,
} DocumentNodeKind;
// clang-format on

// clang-format off
#define DEFINE_DOCUMENT_NODE_FIELDS \
  DocumentNodeKind kind;            \
  Position start;                   \
  Position end;                     \
  Diagnostic* diagnostics;          \
  size_t diagnostics_len;           \
  size_t diagnostics_cap;
// clang-format on

struct _DocumentNode {
  DEFINE_DOCUMENT_NODE_FIELDS;
};

#define FOR_EACH_REF_KIND(V) \
  V(Var)                     \
  V(Task)

// clang-format off
typedef enum {
  kInvalidRefKind = 0,
#define DEFINE_KIND(Name) k##Name##RefKind,
  FOR_EACH_REF_KIND(DEFINE_KIND)
#undef DEFINE_KIND
  kTotalNumberOfRefKinds,
} RefKind;
// clang-format on

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  str_view name;
  RefKind ref_kind;
  DocumentNode* to;
  DocumentNode* from;
} RefNode;

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  str_view value;
} CommandNode;

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  str_view value;
} CommentNode;

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  str_view name;
} TaskNode;

#define FOR_EACH_DOCUMENT_VAR_NODE_KIND(V) \
  V(Scalar)                                \
  V(Shell)                                 \
  V(Map)                                   \
  V(Ref)

// clang-format off
typedef enum {
  kInvalidVarNodeKind = 0,
#define DEFINE_KIND(Name) k##Name##VarNodeKind,
  FOR_EACH_DOCUMENT_VAR_NODE_KIND(DEFINE_KIND)
#undef DEFINE_KIND
  kTotalNumberOfVarNodeKinds,
} VarNodeKind;
// clang-format on

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  str_view name;
  VarNodeKind var_kind;
  union {
    struct {
      DocumentNode* value;
    };
    struct {
      CommandNode* command;
    };
    struct {
      RefNode* ref;
    };
  };
} VarNode;

static inline bool IsNodeKind(DocumentNode* node, const DocumentNodeKind kind) {
  return node && node->kind == kind;
}

#define DEFINE_TYPE_CHECK(Name)                          \
  static inline bool Is##Name##Node(DocumentNode* rhs) { \
    return IsNodeKind(rhs, k##Name##Kind);               \
  }

FOR_EACH_DOCUMENT_NODE_KIND(DEFINE_TYPE_CHECK);
#undef DEFINE_TYPE_CHECK

static inline bool IsVarNodeKind(DocumentNode* rhs, const VarNodeKind kind) {
  return IsVarNode(rhs) && ((VarNode*)rhs)->var_kind == kind;
}

#define DEFINE_TYPE_CHECK(Name)                             \
  static inline bool Is##Name##VarNode(DocumentNode* rhs) { \
    return IsVarNodeKind(rhs, k##Name##VarNodeKind);        \
  }

FOR_EACH_DOCUMENT_VAR_NODE_KIND(DEFINE_TYPE_CHECK)
#undef DEFINE_TYPE_CHECK

static inline bool IsRefNodeKind(DocumentNode* node, const RefKind kind) {
  return IsNodeKind(node, kRefKind) && ((RefNode*)node)->ref_kind == kind;
}

#define DEFINE_TYPE_CHECK(Name)                             \
  static inline bool Is##Name##RefNode(DocumentNode* rhs) { \
    return IsRefNodeKind(rhs, k##Name##RefKind);            \
  }

FOR_EACH_REF_KIND(DEFINE_TYPE_CHECK);
#undef DEFINE_TYPE_CHECK

typedef struct _Document Document;
bool IsFragmentDocument(Document* rhs);
char* GetDocumentPath(Document* doc);
uint64_t GetNumberOfTasksInDocument(Document* rhs);
TaskNode* GetDocumentTaskAt(Document* doc, const uint64_t idx);
uint64_t GetNumberOfCommentsInDocument(Document* rhs);
CommentNode* GetDocumentCommentAt(Document* doc, const uint64_t idx);

Diagnostic* NewDiagnosticForNode(DocumentNode* node, const DiagnosticLevel level, const Position start,
                                 const Position end, const char* fmt, ...);

#define DEFINE_NEW_DIAGNOSTIC(Name)                                                                                    \
  static inline Diagnostic* New##Name##DiagnosticForNode(DocumentNode* node, const Position start, const Position end, \
                                                         const char* fmt, ...) {                                       \
    va_list args;                                                                                                      \
    va_start(args, fmt);                                                                                               \
    Diagnostic* diagnostic = NewDiagnosticForNode(node, k##Name##Level, start, end, fmt, args);                        \
    va_end(args);                                                                                                      \
    return diagnostic;                                                                                                 \
  }
FOR_EACH_DIAGNOSTIC_LEVEL(DEFINE_NEW_DIAGNOSTIC);
#undef DEFINE_NEW_DIAGNOSTIC

static inline bool IsDiagnosticLevel(Diagnostic* lhs, const DiagnosticLevel rhs) {
  return lhs && lhs->level == rhs;
}

#define DEFINE_LEVEL_CHECK(Name)                             \
  static inline bool Is##Name##Diagnostic(Diagnostic* rhs) { \
    return IsDiagnosticLevel(rhs, k##Name##Level);           \
  }

FOR_EACH_DIAGNOSTIC_LEVEL(DEFINE_LEVEL_CHECK)
#undef DEFINE_LEVEL_CHECK

Document* NewDocumentNode();
uint64_t GetNumberOfDiagnosticsForNode(DocumentNode* node);
Diagnostic* GetNodeDiagnosticAt(DocumentNode* node, const uint64_t idx);
bool NodeHasDiagnostics(DocumentNode* rhs);
bool NodeHasDiagnosticsForLevel(DocumentNode* rhs, const DiagnosticLevel level);

#define DEFINE_LEVEL_CHECK(Name)                                 \
  static inline bool Has##Name##Diagnostics(DocumentNode* rhs) { \
    return NodeHasDiagnosticsForLevel(rhs, k##Name##Level);      \
  }

FOR_EACH_DIAGNOSTIC_LEVEL(DEFINE_LEVEL_CHECK)
#undef DEFINE_LEVEL_CHECK

typedef bool (*DiagnosticVisitor)(uint64_t, Diagnostic*, void*);
void VisitNodeDiagnostics(DocumentNode* node, DiagnosticVisitor vis, void* data);

typedef bool (*DiagnosticPredicate)(const uint64_t, Diagnostic*, void*);
void VisitNodeDiagnosticsMatching(DocumentNode* node, DiagnosticPredicate predicate, DiagnosticVisitor vis, void* data);

#define _DEFINE_VISIT_DOCUMENT_FIELD_VALUES(Name, Type)                      \
  typedef bool (*Type##Visitor)(uint64_t, Type*, void*);                     \
  void VisitDocument##Name##s(Document* doc, Type##Visitor vis, void* data); \
  typedef bool (*Type##Predicate)(const uint64_t, Type*, void*);             \
  void VisitDocument##Name##sMatching(Document* doc, Type##Predicate predicate, Type##Visitor vis, void* data);

#define DEFINE_VISIT_DOCUMENT_FIELD_VALUES(Name) _DEFINE_VISIT_DOCUMENT_FIELD_VALUES(Name, Name##Node)

DEFINE_VISIT_DOCUMENT_FIELD_VALUES(Comment);
DEFINE_VISIT_DOCUMENT_FIELD_VALUES(Task);
DEFINE_VISIT_DOCUMENT_FIELD_VALUES(Var);

#undef DEFINE_VISIT_DOCUMENT_FIELD_VALUES
#undef _DEFINE_VISIT_DOCUMENT_FIELD_VALUES

void FreeDocumentNode(DocumentNode* node);

typedef struct {
  bool success;
  union {
    char* msg;
    Document* doc;
  };
} TaskfileParseResult;

TaskfileParseResult ParseTaskfileDocument(const char* data, const size_t data_len);

static inline bool TaskfileParseResultIsOk(TaskfileParseResult* rhs) {
  return rhs && rhs->success;
}

char* TaskfileParseResultToStr(TaskfileParseResult* rhs);
void FreeTaskfileParseResult(TaskfileParseResult* rhs);

#ifdef __cplusplus
};
#endif  // __cplusplus
// NOLINTEND(modernize-use-using,modernize-use-trailing-return-type,cppcoreguidelines-pro-type-cstyle-cast)

#endif  // TASKFILE_PARSER_H
