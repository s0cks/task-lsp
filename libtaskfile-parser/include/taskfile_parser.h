#ifndef TASKFILE_PARSER_H
#define TASKFILE_PARSER_H

// NOLINTBEGIN(modernize-use-using,modernize-use-trailing-return-type,cppcoreguidelines-pro-type-cstyle-cast)
#ifdef __cplusplus
#include <iostream>

extern "C" {
#endif  // __cplusplus

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// ╭───────────╮
// │ ShellOpts │
// ╰───────────╯

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
// ──────────────────────────────────────────────────────────────────────

// ╭────────╮
// │ ShOpts │
// ╰────────╯

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
// ──────────────────────────────────────────────────────────────────────

#define DEFINE_SEQ(Name, Type) \
  typedef struct {             \
    Type* values;              \
    size_t len;                \
    size_t cap;                \
  } Name##Seq;

#define DEFINE_NODE_SEQ(Type) DEFINE_SEQ(Type, Type##Node)

#define DEFINE_VISITOR(Name, Type)                       \
  typedef bool (*Name##Visitor)(uint64_t, Type*, void*); \
  typedef bool (*Name##Predicate)(Type*, void*);

#define DEFINE_NODE_VISITOR(Name) DEFINE_VISITOR(Name, Name##Node)

// ╭────────╮
// │ Method │
// ╰────────╯
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
// ──────────────────────────────────────────────────────────────────────

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

static inline bool StrViewIsEmpty(const StrView view) {
  return view.start == NULL || view.len == 0;
}

static inline bool StrViewEquals(const StrView a, const StrView b) {
  if (a.len != b.len)
    return false;

  return a.len == 0 || memcmp(a.start, b.start, a.len) == 0;
}

static inline bool StrViewEqualsCStr(const StrView a, const char* b) {
  const size_t blen = strlen(b);
  if (a.len != blen)
    return false;
  return a.len == 0 || memcmp(a.start, b, a.len) == 0;
}

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
  V(PipelineExpr)                      \
  V(Command)                           \
  V(Var)                               \
  V(Task)                              \
  V(Comment)                           \
  V(String)                            \
  V(Bool)                              \
  V(Number)                            \
  V(Null)                              \
  V(MapEntry)                          \
  V(Map)                               \
  V(Include)                           \
  V(Precondition)                      \
  V(If)                                \
  V(Set)                               \
  V(For)                               \
  V(Defer)                             \
  V(Output)                            \
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
DEFINE_VISITOR(Diagnostic, Diagnostic);
DEFINE_SEQ(Diagnostic, Diagnostic);

typedef struct _CommentNode CommentNode;
typedef struct {
  CommentNode* values;
  size_t len;
  size_t cap;
} CommentSeq;

typedef struct _VarNode VarNode;
typedef struct {
  VarNode* values;
  size_t len;
  size_t cap;
} VarSeq;

#define FOR_EACH_SCALAR_STYLE(V) \
  V(None)                        \
  V(Plain)                       \
  V(SingleQuoted)                \
  V(DoubleQuoted)                \
  V(LiteralBlock)                \
  V(FoldedBlock)

// clang-format off
typedef enum {
#define DEFINE_STYLE(Name) k##Name##ScalarStyle,
  FOR_EACH_SCALAR_STYLE(DEFINE_STYLE)
#undef DEFINE_STYLE
  kTotalNumberOfScalarStyles,
} ScalarStyle;
// clang-format on

const char* ScalarStyleGetName(ScalarStyle style);

static inline bool IsQuotedScalarStyle(const ScalarStyle style) {
  return style == kSingleQuotedScalarStyle || style == kDoubleQuotedScalarStyle;
}

static inline bool IsBlockScalarStyle(const ScalarStyle style) {
  return style == kLiteralBlockScalarStyle || style == kFoldedBlockScalarStyle;
}

#define FOR_EACH_CHOMP_STYLE(V) \
  V(Clip)                       \
  V(Strip)                      \
  V(Keep)

// clang-format off
typedef enum {
#define DEFINE_CHOMP(Name) k##Name##Chomp,
  FOR_EACH_CHOMP_STYLE(DEFINE_CHOMP)
#undef DEFINE_CHOMP
  kTotalNumberOfChompStyles,
  kDefaultChomp = kClipChomp,
} ChompStyle;
// clang-format on

typedef uint8_t NodeStatus;

enum {
  kNodeOk = 0,
  kNodeIncomplete = 1 << 0,
  kNodeError = 1 << 1,
};

// clang-format off
#define DEFINE_DOCUMENT_NODE_FIELDS \
  DocumentNodeKind kind;            \
  Pos start;                        \
  Pos end;                          \
  Range key_range;                  \
  Range value_range;                \
  NodeStatus status;                \
  ScalarStyle style;                \
  int indent;                       \
  int depth;                        \
  int leading_ws;                   \
  int trailing_ws;                  \
  CommentNode* trailing_comment;    \
  DiagnosticSeq diagnostics;        \
  CommentSeq comments;
// clang-format on

struct _DocumentNode {
  DEFINE_DOCUMENT_NODE_FIELDS;
};

static inline bool HasNodeStatus(DocumentNode* node, const NodeStatus rhs) {
  return node && (node->status & rhs) == rhs;
}

static inline void SetNodeStatus(DocumentNode* node, const NodeStatus rhs) {
  if (!node)
    return;
  node->status |= rhs;
}

static inline bool IsNodeIncomplete(DocumentNode* rhs) {
  return HasNodeStatus(rhs, kNodeIncomplete);
}

static inline bool IsNodeError(DocumentNode* rhs) {
  return HasNodeStatus(rhs, kNodeError);
}

static inline void MarkNodeIncomplete(DocumentNode* rhs) {
  return SetNodeStatus(rhs, kNodeIncomplete);
}

static inline void MarkNodeError(DocumentNode* rhs) {
  return SetNodeStatus(rhs, kNodeError);
}

static inline bool NodeHasTrailingComment(DocumentNode* rhs) {
  return rhs && rhs->trailing_comment != NULL;
}

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  StrView name;
  StrView args;
} PipelineExprNode;
DEFINE_NODE_VISITOR(PipelineExpr);
DEFINE_NODE_SEQ(PipelineExpr);

#define FOR_EACH_REF_KIND(V) \
  V(Var)                     \
  V(Env)                     \
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
  PipelineExprSeq pipeline;
} RefNode;
DEFINE_NODE_VISITOR(Ref);
DEFINE_NODE_SEQ(Ref);

static inline bool RefHasPipeline(RefNode* rhs) {
  return rhs && rhs->pipeline.len > 0;
}

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
  bool value;
} BoolNode;
DEFINE_NODE_VISITOR(Bool);

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  StrView raw;
  double value;
} NumberNode;
DEFINE_NODE_VISITOR(Number);

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
} NullNode;
DEFINE_NODE_VISITOR(Null);

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  StrView key;
  DocumentNode* value;
} MapEntryNode;
DEFINE_NODE_VISITOR(MapEntry);
DEFINE_NODE_SEQ(MapEntry);

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  MapEntrySeq entries;
} MapNode;
DEFINE_NODE_VISITOR(Map);
DEFINE_NODE_SEQ(Map);

// ╭─────────╮
// │ If Node │
// ╰─────────╯

// clang-format off
#define FOR_EACH_CONDITION_KIND(V) \
  V(Shell)                       \
  V(GoTemplate)

typedef enum {
  kInvalidCond = 0,
#define DEFINE_KIND(Name) k##Name##Cond,
  FOR_EACH_CONDITION_KIND(DEFINE_KIND)
#undef DEFINE_KIND
  kTotalNumberOfConditionKinds,
} ConditionKind;
// clang-format on

typedef struct {
  ConditionKind kind;
  StringNode* expr;
} Condition;

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  Condition cond;
} IfNode;
// ──────────────────────────────────────────────────────────────────────

// ╭──────────────╮
// │ Include Node │
// ╰──────────────╯

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
  VarSeq vars;
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
// ──────────────────────────────────────────────────────────────────────

// ╭──────────────╮
// │ Command Node │
// ╰──────────────╯
#define FOR_EACH_FOR_KIND(V) \
  V(List)                    \
  V(Attribute)               \
  V(Var)                     \
  V(Matrix)

// clang-format off
typedef enum {
  kInvalidForKind = 0,
#define DEFINE_KIND(Name) k##Name##ForKind,
  FOR_EACH_FOR_KIND(DEFINE_KIND)
#undef DEFINE_KIND
  kTotalNumberOfForKinds,
} ForKind;
// clang-format on

#define FOR_EACH_FOR_ATTRIBUTE(V) \
  V(Sources)                      \
  V(Generates)

// clang-format off
typedef enum {
  kInvalidForAttribute = 0,
#define DEFINE_ATTR(Name) k##Name##ForAttribute,
  FOR_EACH_FOR_ATTRIBUTE(DEFINE_ATTR)
#undef DEFINE_ATTR
  kTotalNumberOfForAttributes,
} ForAttribute;
// clang-format on

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  ForKind for_kind;
  ForAttribute attribute;
  StringSeq items;
  RefNode var;
  StrView split;
  StrView as;
  MapNode* matrix;
} ForNode;
DEFINE_NODE_VISITOR(For);

static inline bool ForHasSplit(ForNode* rhs) {
  return rhs && !StrViewIsEmpty(rhs->split);
}

uint64_t GetNumberOfItemsInFor(ForNode* rhs);
StringNode* GetForItemAt(ForNode* node, uint64_t idx);
void VisitForItems(ForNode* node, StringVisitor vis, void* data);

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  bool is_task_call;
  RefNode task_call;
  StringNode* cmd;
} DeferNode;
DEFINE_NODE_VISITOR(Defer);

static inline bool IsDeferTaskCall(DeferNode* rhs) {
  return rhs && rhs->is_task_call;
}

#define FOR_EACH_OUTPUT_KIND(V) \
  V(Interleaved)                \
  V(Group)                      \
  V(Prefixed)

// clang-format off
typedef enum {
  kInvalidOutputKind = 0,
#define DEFINE_KIND(Name) k##Name##OutputKind,
  FOR_EACH_OUTPUT_KIND(DEFINE_KIND)
#undef DEFINE_KIND
  kTotalNumberOfOutputKinds,
  kDefaultOutputKind = kInterleavedOutputKind,
} OutputKind;
// clang-format on

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  OutputKind output_kind;
  StrView begin;
  StrView end_template;
  bool error_only;
} OutputNode;
DEFINE_NODE_VISITOR(Output);

typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  StringNode cmd;
  ShellOpts* set;
  ShOpts* shopt;
  bool silent;
  bool ignore_error;
  StringSeq platforms;
  StrView timeout;  // TODO(@s0cks): convert to Time expr node
  RefNode task_call;
  StringNode* if_expr;
  ForNode* for_each;
  DeferNode* defer;
} CommandNode;
DEFINE_NODE_VISITOR(Command);
DEFINE_NODE_SEQ(Command);
// ──────────────────────────────────────────────────────────────────────

static inline bool IsCommandTaskCall(CommandNode* rhs) {
  return rhs && rhs->task_call.name.start != NULL;
}

// ╭───────────────────╮
// │ Precondition Node │
// ╰───────────────────╯
typedef struct {
  DEFINE_DOCUMENT_NODE_FIELDS;
  CommandNode* command;
  StringNode* message;
} PreconditionNode;
DEFINE_NODE_VISITOR(Precondition);
DEFINE_NODE_SEQ(Precondition);
// ──────────────────────────────────────────────────────────────────────

// ╭──────────────╮
// │ Comment Node │
// ╰──────────────╯
struct _CommentNode {
  DEFINE_DOCUMENT_NODE_FIELDS;
  StrView value;
};
DEFINE_NODE_VISITOR(Comment);
// ──────────────────────────────────────────────────────────────────────

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

struct _VarNode {
  DEFINE_DOCUMENT_NODE_FIELDS;
  StrView name;
  VarNodeKind var_kind;
  bool secret;
  DocumentNode* value;
  CommandNode* command;
  RefNode* ref;
};
DEFINE_NODE_VISITOR(Var);

static inline bool IsVarSecret(VarNode* rhs) {
  return rhs && rhs->secret;
}

// ╭───────────╮
// │ Task Node │
// ╰───────────╯
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
  kInteractiveFlag = 1 << 4,
  kPrefixedFlag = 1 << 5,
  kIgnoreErrorFlag = 1 << 6,
  kWatchFlag = 1 << 7,
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
  StrViewSeq reqs;

  StrView label;
  StrView prefix;
  bool is_short_form;
  CommandSeq cmds;
  RefSeq deps;
  VarSeq vars;
  VarSeq env;

  CommandSeq status_cmds;
  PreconditionSeq preconditions;
  StringSeq aliases;
  StringSeq prompts;
  StringSeq sources;
  StringSeq generates;
  StringSeq requires_vars;

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
DEFINE_TASK_FLAG(Interactive);
DEFINE_TASK_FLAG(Prefixed);
DEFINE_TASK_FLAG(IgnoreError);
DEFINE_TASK_FLAG(Watch);

static inline bool IsTaskShortForm(TaskNode* rhs) {
  return rhs && rhs->is_short_form;
}

uint64_t GetNumberOfDotenvsInTask(TaskNode* rhs);
StringNode* GetTaskDotenvAt(TaskNode* node, uint64_t idx);
void VisitTaskDotenvs(TaskNode* node, StringVisitor vis, void* data);
void VisitTaskDeps(TaskNode* node, RefVisitor vis, void* data);

static inline bool TaskHasDotenvs(TaskNode* rhs) {
  return GetNumberOfDotenvsInTask(rhs) > 0;
}

#define FOR_EACH_TASK_STRING_SEQ(V)           \
  V(Alias, Aliases, aliases)                  \
  V(Prompt, Prompts, prompts)                 \
  V(Source, Sources, sources)                 \
  V(Generate, Generates, generates)           \
  V(RequiredVar, RequiredVars, requires_vars) \
  V(Platform, Platforms, platforms)

#define DEFINE_TASK_STRING_SEQ_ACCESSORS(Singular, Plural, Field)        \
  uint64_t GetNumberOf##Plural##InTask(TaskNode* rhs);                   \
  StringNode* GetTask##Singular##At(TaskNode* node, uint64_t idx);       \
  void VisitTask##Plural(TaskNode* node, StringVisitor vis, void* data); \
  static inline bool TaskHas##Plural(TaskNode* rhs) {                    \
    return GetNumberOf##Plural##InTask(rhs) > 0;                         \
  }

FOR_EACH_TASK_STRING_SEQ(DEFINE_TASK_STRING_SEQ_ACCESSORS)
#undef DEFINE_TASK_STRING_SEQ_ACCESSORS
// ──────────────────────────────────────────────────────────────────────

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

#define FOR_EACH_NEWLINE_STYLE(V) \
  V(Unknown)                      \
  V(Lf)                           \
  V(CrLf)                         \
  V(Mixed)

// clang-format off
typedef enum {
#define DEFINE_NEWLINE(Name) k##Name##Newline,
  FOR_EACH_NEWLINE_STYLE(DEFINE_NEWLINE)
#undef DEFINE_NEWLINE
  kTotalNumberOfNewlineStyles,
} NewlineStyle;
// clang-format on

const char* NewlineStyleGetName(NewlineStyle style);

typedef struct {
  int* values;
  size_t len;
  size_t cap;
} LineSeq;

typedef struct _Document Document;
void FreeDocument(Document*);

OutputNode* GetDocumentOutput(Document* doc);
NewlineStyle GetDocumentNewlineStyle(Document* doc);
int GetDocumentIndentWidth(Document* doc);
bool DocumentUsesTabsForIndent(Document* doc);

uint64_t GetNumberOfBlankLinesInDocument(Document* doc);
int GetDocumentBlankLineAt(Document* doc, uint64_t idx);
bool IsDocumentLineBlank(Document* doc, int line);

bool IsFragmentDocument(Document* rhs);
bool IsPartialDocument(Document* rhs);
char* GetDocumentPath(Document* doc);
StrView GetDocumentVersion(Document* doc);

uint64_t GetNumberOfTasksInDocument(Document* rhs);
TaskNode* GetDocumentTaskAt(Document* doc, const uint64_t idx);
TaskNode* FindDocumentTask(Document* doc, const StrView name);
void VisitDocumentTasks(Document* doc, TaskVisitor vis, void* data);
void VisitDocumentTasksMatch(Document* doc, TaskPredicate predicate, TaskVisitor vis, void* data);

static inline bool DocumentHasTasks(Document* rhs) {
  return GetNumberOfTasksInDocument(rhs) > 0;
}

uint64_t GetNumberOfVarsInDocument(Document* rhs);
VarNode* GetDocumentVarAt(Document* doc, const uint64_t idx);
VarNode* FindDocumentVar(Document* doc, const StrView name);
VarNode* FindTaskVar(TaskNode* task, const StrView name);

static inline bool DocumentHasVars(Document* rhs) {
  return GetNumberOfVarsInDocument(rhs) > 0;
}

uint64_t GetNumberOfEnvVarsInDocument(Document* rhs);
VarNode* GetDocumentEnvVarAt(Document* doc, const uint64_t idx);
VarNode* FindDocumentEnvVar(Document* doc, const StrView name);
VarNode* FindTaskEnvVar(TaskNode* task, const StrView name);

static inline bool DocumentHasEnvVars(Document* rhs) {
  return GetNumberOfEnvVarsInDocument(rhs) > 0;
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

void VisitNodeDiagnostics(DocumentNode* node, DiagnosticVisitor vis, void* data);
void VisitNodeDiagnosticsMatching(DocumentNode* node, DiagnosticPredicate predicate, DiagnosticVisitor vis, void* data);

uint64_t GetNumberOfCommentsForNode(DocumentNode* node);
CommentNode* GetNodeCommentAt(DocumentNode* node, const uint64_t idx);
void VisitNodeComments(DocumentNode* node, CommentVisitor vis, void* data);
void VisitNodeCommentsMatching(DocumentNode* node, CommentPredicate predicate, CommentVisitor vis, void* data);

static inline bool NodeHasComments(DocumentNode* rhs) {
  return GetNumberOfCommentsForNode(rhs) > 0;
}

#define _DEFINE_VISIT_DOCUMENT_FIELD_VALUES(Name, Type)                      \
  void VisitDocument##Name##s(Document* doc, Type##Visitor vis, void* data); \
  void VisitDocument##Name##sMatching(Document* doc, Type##Predicate predicate, Type##Visitor vis, void* data);

#define DEFINE_VISIT_DOCUMENT_FIELD_VALUES(Name) _DEFINE_VISIT_DOCUMENT_FIELD_VALUES(Name, Name)

DEFINE_VISIT_DOCUMENT_FIELD_VALUES(Comment);
DEFINE_VISIT_DOCUMENT_FIELD_VALUES(Task);
DEFINE_VISIT_DOCUMENT_FIELD_VALUES(Var);

#undef DEFINE_VISIT_DOCUMENT_FIELD_VALUES
#undef _DEFINE_VISIT_DOCUMENT_FIELD_VALUES

// TODO(@s0cks): kVisitStop currently only prunes the current subtree (siblings still
// visit); consider a third kSkip/kContinue/kStop state so Find*-style callers can request
// a full walk abort distinctly from "not interested in this branch."
typedef enum {
  kVisitContinue,
  kVisitStop,
} VisitResult;

typedef VisitResult (*DocumentNodeVisitor)(DocumentNode* node, void* data);

void VisitDocument(Document* doc, DocumentNodeVisitor before, DocumentNodeVisitor after, void* data);
void VisitNode(DocumentNode* node, DocumentNodeVisitor before, DocumentNodeVisitor after, void* data);

DocumentNode* FindNodeAtPosition(Document* doc, Pos pos);

void FreeDocumentNode(DocumentNode* node);

// ╭───────╮
// │ Parse │
// ╰───────╯
typedef struct {
  bool success;
  Document* doc;
  char* msg;
} TaskfileParseResult;

TaskfileParseResult ParseTaskfileDocumentStr(const char* data, const size_t data_len);

static inline bool TaskfileParseResultIsOk(TaskfileParseResult* rhs) {
  return rhs && rhs->success;
}

char* TaskfileParseResultToStr(TaskfileParseResult* rhs);
void FreeTaskfileParseResult(TaskfileParseResult* rhs);
// ──────────────────────────────────────────────────────────────────────

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
