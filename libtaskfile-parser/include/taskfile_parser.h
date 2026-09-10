#ifndef TASKFILE_PARSER_H
#define TASKFILE_PARSER_H

// NOLINTBEGIN(modernize-use-using,modernize-use-trailing-return-type,cppcoreguidelines-pro-type-cstyle-cast)
#ifdef __cplusplus
#include <iostream>

extern "C" {
#endif  // __cplusplus

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

// clang-format off
typedef uint8_t ShellOpts;
enum AllShellOpts {
  kNoShellOpts            = 0,
  kAllExportShellOpt      = 1 << 1,
  kErrExitShellOpt        = 1 << 2,
  kNoExecShellOpt         = 1 << 3,
  kNoGlobShellOpt         = 1 << 4,
  kNoUnsetShellOpt        = 1 << 5,
  kXTraceShellOpt         = 1 << 6,
  kPipeFailShellOpt       = 1 << 7,
  kTotalNumberOfShellOpts = 7,
};
// clang-format on

static inline ShellOpts SetShellOpts(const ShellOpts lhs, const ShellOpts rhs) {
  return lhs | rhs;
}

static inline bool TestShellOpts(const ShellOpts lhs, const ShellOpts rhs) {
  return (lhs & rhs) == rhs;
}

// clang-format off
typedef uint8_t ShOpts;
enum AllShOpts {
  kNoShOpts            = 0,
  kExpandAliasesShOpt  = 1 << 1,
  kGlobStarShOpt       = 1 << 2,
  kNullGlobShOpt       = 1 << 3,
  kTotalNumberOfShOpts = 3,
};
// clang-format on

static inline ShOpts SetShOpts(const ShOpts lhs, const ShOpts rhs) {
  return lhs | rhs;
}

static inline bool TestShOpts(const ShOpts lhs, const ShOpts rhs) {
  return (lhs & rhs) == rhs;
}

#define DEFINE_SEQ(Name, Type) \
  typedef struct {             \
    Type* values;              \
    size_t len;                \
    size_t cap;                \
  } Name##Seq;

#define DEFINE_NODE_SEQ(Type) DEFINE_SEQ(Type, Type##Node)

#define DEFINE_NODE_VISITOR(Name)                              \
  typedef bool (*Name##Visitor)(uint64_t, Name##Node*, void*); \
  typedef bool (*Name##Predicate)(Name##Node*, void*);

#define FOR_EACH_METHOD_KIND(V) \
  V(None)                       \
  V(Checksum)                   \
  V(Timestamp)

// clang-format off
typedef enum {
#define DEFINE_KIND(Name) k##Name##MethodKind,
  FOR_EACH_METHOD_KIND(DEFINE_KIND)
#undef DEFINE_KIND
  kTotalNumberOfMethodKinds,
  kDefaultMethodKind = kChecksumMethodKind,
} MethodKind;
// clang-format on

typedef struct _DocumentNode DocumentNode;

typedef struct {
  int row;
  int col;
} Pos;

typedef struct {
  Pos start;
  Pos end;
} Range;

typedef struct _StrView {
  char* start;
  size_t len;

#ifdef __cplusplus

  operator std::string() const {
    return {start, len};
  }

#endif  // __cplusplus
} StrView;

typedef struct {
  StrView* values;
  size_t len;
  size_t cap;
} StrViewSeq;

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

#define FOR_EACH_DOCUMENT_NODE_KIND(V) \
  V(Document)                          \
  V(Ref)                               \
  V(Command)                           \
  V(Var)                               \
  V(Task)                              \
  V(Comment)                           \
  V(String)                            \
  V(Include)                           \
  V(Precondition)                      \
  V(If)                                \
  V(Set)                               \
  V(Diagnostic)

// clang-format off
typedef enum {
  kInvalidDocumentNodeKind = 0,
#define DEFINE_KIND(Name) k##Name##Kind,
  FOR_EACH_DOCUMENT_NODE_KIND(DEFINE_KIND)
#undef DEFINE_KIND
  kTotalNumberOfDocumentNodeKinds,
} DocumentNodeKind;
// clang-format on

typedef struct {
  DiagnosticLevel level;
  Range range;
  char* message;
} Diagnostic;
DEFINE_SEQ(Diagnostic, Diagnostic);

// clang-format off
#define DEFINE_DOCUMENT_NODE_FIELDS \
  DocumentNodeKind kind;            \
  Pos start;                        \
  Pos end;                          \
  DiagnosticSeq diagnostics;
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
  StrView name;
  RefKind ref_kind;
  DocumentNode* to;
  DocumentNode* from;
} RefNode;
DEFINE_NODE_VISITOR(Ref);
DEFINE_NODE_SEQ(Ref);

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  StrView value;
  RefSeq refs;
} StringNode;
DEFINE_NODE_VISITOR(String);
DEFINE_NODE_SEQ(String);

uint64_t GetNumberOfRefsInString(StringNode* rhs);
RefNode* GetStringRefAt(StringNode* node, uint64_t idx);
void VisitStringRefs(StringNode* node, RefVisitor vis, void* data);

static inline bool StringHasRefs(StringNode* rhs) {
  return GetNumberOfRefsInString(rhs) > 0;
}

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  StringNode* expr;
  // TODO(@s0cks): should prolly handle sh vs Go templates
} IfNode;

// TODO(@s0cks): compress to IncludeFlags
typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  bool optional;
  bool flatten;
  bool internal;
  StrView dir;
  StrView checksum;
  StrView taskfile;
  StringSeq aliases;
  StringSeq excludes;

  // TODO(@s0cks): handle vars
} IncludeNode;
DEFINE_NODE_VISITOR(Include);
DEFINE_NODE_SEQ(Include);

uint64_t GetNumberOfIncludeAliases(IncludeNode* rhs);
StringNode* GetIncludeAliasAt(IncludeNode* node, uint64_t idx);
void VisitIncludeAliases(IncludeNode* node, StringVisitor vis, void* data);

static inline bool IncludeHasAliases(IncludeNode* rhs) {
  return GetNumberOfIncludeAliases(rhs) > 0;
}

uint64_t GetNumberOfIncludeExcludes(IncludeNode* rhs);
StringNode* GetIncludeExcludeAt(IncludeNode* node, uint64_t idx);
void VisitIncludeExcludes(IncludeNode* node, StringVisitor vis, void* data);

static inline bool IncludeHasExcludes(IncludeNode* rhs) {
  return GetNumberOfIncludeExcludes(rhs) > 0;
}

// TODO(@s0cks): handle loops

// TODO(@s0cks): handle defer

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  StringNode cmd;
  ShellOpts* set;
  ShOpts* shopt;
  bool silent;
  bool ignore_error;
  StrViewSeq platforms;
  StrView timeout;  // TODO(@s0cks): convert to Time expr node
} CommandNode;
DEFINE_NODE_VISITOR(Command);
DEFINE_NODE_SEQ(Command);

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  CommandNode* command;
  StringNode* message;
} PreconditionNode;
DEFINE_NODE_VISITOR(Precondition);
DEFINE_NODE_SEQ(Precondition);

void VisitPreconditions(PreconditionSeq* seq, PreconditionVisitor vis, void* data);

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  StrView value;
} CommentNode;
DEFINE_NODE_VISITOR(Comment);
DEFINE_NODE_SEQ(Comment);

#define FOR_EACH_TASK_RUN_MODE(V) \
  V(Always)                       \
  V(Once)                         \
  V(WhenChanged)

// clang-format off
typedef enum {
  kInvalidTaskRunMode,
#define DEFINE_KIND(Name) kTaskRun##Name##Mode,
  FOR_EACH_TASK_RUN_MODE(DEFINE_KIND)
#undef DEFINE_KIND
  kTotalNumberOfTaskRunModes,
  kDefaultTaskRunMode = kTaskRunAlwaysMode,
} TaskRunMode;
// clang-format on

typedef uint8_t TaskFlags;

enum {
  kNoTaskFlags = 0,
  kSilentFlag = 1 << 1,
  kGitignoreFlag = 1 << 2,
  kInternalFlag = 1 << 3,
};

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  StrView name;
  StrView desc;
  StrView dir;
  StrView summary;
  TaskFlags flags;
  TaskRunMode mode;
  MethodKind method;
  StrView interval;
  ShellOpts* set;
  ShOpts* shopt;

  CommandSeq status;
  PreconditionSeq preconditions;

  // TODO(@s0cks):
  // - add aliases
  // - add prompts
  // - add sources
  // - add generates

  StringSeq dotenvs;
  StringSeq platforms;
} TaskNode;
DEFINE_NODE_VISITOR(Task);
DEFINE_NODE_SEQ(Task);

static inline bool HasTaskFlags(TaskNode* node, const TaskFlags rhs) {
  return node && (node->flags & rhs) == rhs;
}

static inline void SetTaskFlags(TaskNode* node, TaskFlags rhs) {
  if (!node)
    return;
  node->flags = rhs;
}

static inline void ClearTaskFlags(TaskNode* node) {
  return SetTaskFlags(node, kNoTaskFlags);
}

#define DEFINE_TASK_FLAG(Name)                              \
  static inline void SetTask##Name##Flag(TaskNode* node) {  \
    if (!node)                                              \
      return;                                               \
    return SetTaskFlags(node, node->flags | k##Name##Flag); \
  }                                                         \
  static inline bool HasTask##Name##Flag(TaskNode* node) {  \
    return node && HasTaskFlags(node, k##Name##Flag);       \
  }                                                         \
  static inline bool IsTask##Name(TaskNode* rhs) {          \
    return HasTask##Name##Flag(rhs);                        \
  }

DEFINE_TASK_FLAG(Silent);
DEFINE_TASK_FLAG(Gitignore);
DEFINE_TASK_FLAG(Internal);

uint64_t GetNumberOfDotenvsInTask(TaskNode* rhs);
StringNode* GetTaskDotenvAt(TaskNode* node, uint64_t idx);
void VisitTaskDotenvs(TaskNode* node, StringVisitor vis, void* data);

static inline bool TaskHasDotenvs(TaskNode* rhs) {
  return GetNumberOfDotenvsInTask(rhs) > 0;
}

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
  StrView name;
  VarNodeKind var_kind;
  bool secret;
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
DEFINE_NODE_VISITOR(Var);
DEFINE_NODE_SEQ(Var);

static inline bool IsVarSecret(VarNode* rhs) {
  return rhs && rhs->secret;
}

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

static inline bool DocumentHasTasks(Document* rhs) {
  return GetNumberOfTasksInDocument(rhs) > 0;
}

uint64_t GetNumberOfCommentsInDocument(Document* rhs);
CommentNode* GetDocumentCommentAt(Document* doc, const uint64_t idx);

uint64_t GetNumberOfIncludesInDocument(Document* doc);
IncludeNode* GetDocumentIncludeAt(Document* doc, const uint64_t idx);
void VisitDocumentIncludes(Document* doc, IncludeVisitor vis, void* data);

static inline bool DocumentHasIncludes(Document* rhs) {
  return GetNumberOfIncludesInDocument(rhs) > 0;
}

uint64_t GetNumberOfDotenvsInDocument(Document* doc);
StringNode* GetDocumentDotenvAt(Document* doc, uint64_t idx);
void VisitDocumentDotenvs(Document* doc, StringVisitor vis, void* data);

static inline bool DocumentHasDotenvs(Document* rhs) {
  return GetNumberOfDotenvsInDocument(rhs) > 0;
}

Diagnostic* NewDiagnosticForNode(DocumentNode* node, const DiagnosticLevel level, const Range range, const char* fmt,
                                 ...);

#define DEFINE_NEW_DIAGNOSTIC(Name)                                                                              \
  static inline Diagnostic* New##Name##DiagnosticForNode(DocumentNode* node, const Range range, const char* fmt, \
                                                         ...) {                                                  \
    va_list args;                                                                                                \
    va_start(args, fmt);                                                                                         \
    Diagnostic* diagnostic = NewDiagnosticForNode(node, k##Name##Level, range, fmt, args);                       \
    va_end(args);                                                                                                \
    return diagnostic;                                                                                           \
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

TaskfileParseResult ParseTaskfileDocumentStr(const char* data, const size_t data_len);

static inline bool TaskfileParseResultIsOk(TaskfileParseResult* rhs) {
  return rhs && rhs->success;
}

char* TaskfileParseResultToStr(TaskfileParseResult* rhs);
void FreeTaskfileParseResult(TaskfileParseResult* rhs);

#ifdef __cplusplus
};

static inline auto operator<<(std::ostream& stream, const Pos& rhs) -> std::ostream& {
  stream << "Pos{";
  stream << "row=" << rhs.row << ", ";
  stream << "col=" << rhs.col;
  stream << "}";
  return stream;
}

static inline auto operator<<(std::ostream& stream, const Range& rhs) -> std::ostream& {
  stream << "Range{";
  stream << "start=" << rhs.start << ", ";
  stream << "end=" << rhs.end;
  stream << "}";
  return stream;
}

static inline auto operator<<(std::ostream& stream, const StrView& rhs) -> std::ostream& {
  return stream << std::string(rhs.start, rhs.len);
}
#endif  // __cplusplus
// NOLINTEND(modernize-use-using,modernize-use-trailing-return-type,cppcoreguidelines-pro-type-cstyle-cast)

#endif  // TASKFILE_PARSER_H
