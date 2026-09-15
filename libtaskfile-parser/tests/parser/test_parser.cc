#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

#include "seq.h"
#include "taskfile_parser.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)

class TestParser : public ::testing::Test {};

static inline auto IsParseOk(const TaskfileParseResult& rhs) -> ::testing::AssertionResult {
  if (!rhs.success)
    return ::testing::AssertionFailure() << "parse result is not successful";
  return ::testing::AssertionSuccess();
}

static inline auto IsParseError(const TaskfileParseResult& result) -> ::testing::AssertionResult {
  if (result.success)
    return ::testing::AssertionFailure() << "result is successful, expected error";
  return ::testing::AssertionSuccess();
}

static inline auto IsParseError(const TaskfileParseResult& result, const std::string message)
    -> ::testing::AssertionResult {
  if (result.success)
    return ::testing::AssertionFailure() << "result is successful, expected error";

  const std::string_view msg(result.msg);
  if (message != msg)
    return ::testing::AssertionFailure() << "error result message is invalid, expected `" << message << "` got `"
                                         << result.msg << "`";

  return ::testing::AssertionSuccess();
}

static inline auto HasDiagnostic(const TaskfileParseResult& result, DiagnosticPredicate filter)
    -> ::testing::AssertionResult {
  if (!result.success)
    return ::testing::AssertionFailure() << "expected parse result to be success, but had error: "
                                         << std::string_view(result.msg);

  return ::testing::AssertionSuccess();
}

#define FOR_EACH_PARE_ERROR_MESSAGE(V) \
  V(NotImplemented, "not implemented") \
  V(EmptyDocument, "document is empty")

#define DEFINE_PARSE_ERROR_MESSAGE_ASSERT(Name, Message)                                                     \
  static inline auto Is##Name##ParseError(const TaskfileParseResult& result) -> ::testing::AssertionResult { \
    return IsParseError(result, (Message));                                                                  \
  }

FOR_EACH_PARE_ERROR_MESSAGE(DEFINE_PARSE_ERROR_MESSAGE_ASSERT)
#undef DEFINE_PARSE_ERROR_MESSAGE_ASSERT

TEST_F(TestParser, Test_Parse_DocumentEmpty) {
  TaskfileParseResult result = ParseTaskfileDocumentStr(NULL, 0);
  ASSERT_TRUE(IsEmptyDocumentParseError(result));
}

TEST_F(TestParser, Test_Parse_SimpleTaskWithDescAndCmds) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    desc: Build the project\n"
      "    cmds:\n"
      "      - go build ./...\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  Document* doc = result.doc;
  ASSERT_EQ(GetNumberOfTasksInDocument(doc), 1);
  TaskNode* task = GetDocumentTaskAt(doc, 0);
  ASSERT_EQ(std::string(task->name), "build");
  ASSERT_EQ(std::string(task->desc), "Build the project");
  ASSERT_EQ(GetNumberOfCommandsInSeq(&task->cmds), 1);
  CommandNode* cmd = GetCommandInSeqAt(&task->cmds, 0);
  ASSERT_FALSE(IsCommandTaskCall(cmd));
  ASSERT_EQ(std::string(cmd->cmd.value), "go build ./...");
}

TEST_F(TestParser, Test_Parse_DuplicateTaskMarksDocumentPartial) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - echo one\n"
      "  build:\n"
      "    cmds:\n"
      "      - echo two\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_EQ(GetNumberOfTasksInDocument(result.doc), 1);
  ASSERT_TRUE(IsPartialDocument(result.doc));
}

TEST_F(TestParser, Test_Parse_DepsInlineAndBlockResolveAgainstEarlierTasks) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - echo build\n"
      "  test:\n"
      "    deps: [build, missing]\n"
      "    cmds:\n"
      "      - echo test\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* test_task = GetDocumentTaskAt(result.doc, 1);
  ASSERT_EQ(std::string(test_task->name), "test");
  ASSERT_EQ(GetNumberOfRefsInSeq(&test_task->deps), 2);

  RefNode* resolved = GetRefInSeqAt(&test_task->deps, 0);
  ASSERT_EQ(std::string(resolved->name), "build");
  ASSERT_TRUE(resolved->to != nullptr);
  ASSERT_TRUE(IsTaskNode(resolved->to));

  RefNode* unresolved = GetRefInSeqAt(&test_task->deps, 1);
  ASSERT_EQ(std::string(unresolved->name), "missing");
  ASSERT_TRUE(unresolved->to == nullptr);
}

TEST_F(TestParser, Test_Parse_TaskCallInCmdsResolves) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - echo build\n"
      "  test:\n"
      "    cmds:\n"
      "      - task: build\n"
      "      - echo done\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* test_task = GetDocumentTaskAt(result.doc, 1);
  ASSERT_EQ(GetNumberOfCommandsInSeq(&test_task->cmds), 2);

  CommandNode* call = GetCommandInSeqAt(&test_task->cmds, 0);
  ASSERT_TRUE(IsCommandTaskCall(call));
  ASSERT_EQ(std::string(call->task_call.name), "build");
  ASSERT_TRUE(call->task_call.to != nullptr);

  CommandNode* plain = GetCommandInSeqAt(&test_task->cmds, 1);
  ASSERT_FALSE(IsCommandTaskCall(plain));
  ASSERT_EQ(std::string(plain->cmd.value), "echo done");
}

TEST_F(TestParser, Test_Parse_GlobalScalarVarsClassifyLiteralKinds) {
  static const char* kDoc =
      "vars:\n"
      "  APP_NAME: myapp\n"
      "  DEBUG: true\n"
      "  RETRIES: 3\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_EQ(GetNumberOfVarsInDocument(result.doc), 3);

  VarNode* app_name = GetDocumentVarAt(result.doc, 0);
  ASSERT_EQ(app_name->var_kind, kScalarVarNodeKind);
  ASSERT_TRUE(IsStringNode(app_name->value));
  ASSERT_EQ(std::string(((StringNode*)app_name->value)->value), "myapp");

  VarNode* debug = GetDocumentVarAt(result.doc, 1);
  ASSERT_TRUE(IsBoolNode(debug->value));
  ASSERT_TRUE(((BoolNode*)debug->value)->value);

  VarNode* retries = GetDocumentVarAt(result.doc, 2);
  ASSERT_TRUE(IsNumberNode(retries->value));
  ASSERT_EQ(((NumberNode*)retries->value)->value, 3.0);
}

TEST_F(TestParser, Test_Parse_ShellRefAndMapVars) {
  static const char* kDoc =
      "vars:\n"
      "  VERSION: 1.0.0\n"
      "  COMMIT_HASH:\n"
      "    sh: git rev-parse HEAD\n"
      "  BUILD_VERSION:\n"
      "    ref: .VERSION\n"
      "  CONFIG:\n"
      "    map:\n"
      "      database: postgres\n"
      "      port: 5432\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  VarNode* commit_hash = GetDocumentVarAt(result.doc, 1);
  ASSERT_EQ(commit_hash->var_kind, kShellVarNodeKind);
  ASSERT_EQ(std::string(commit_hash->command->cmd.value), "git rev-parse HEAD");

  VarNode* build_version = GetDocumentVarAt(result.doc, 2);
  ASSERT_EQ(build_version->var_kind, kRefVarNodeKind);
  ASSERT_EQ(std::string(build_version->ref->name), "VERSION");
  ASSERT_TRUE(build_version->ref->to != nullptr);

  VarNode* config = GetDocumentVarAt(result.doc, 3);
  ASSERT_EQ(config->var_kind, kMapVarNodeKind);
  MapNode* map = (MapNode*)config->value;
  ASSERT_EQ(GetNumberOfMapEntrysInSeq(&map->entries), 2);
  ASSERT_EQ(std::string(GetMapEntryInSeqAt(&map->entries, 0)->key), "database");
}

TEST_F(TestParser, Test_Parse_TaskLocalVarShadowsGlobal) {
  static const char* kDoc =
      "vars:\n"
      "  GREETING: hello\n"
      "tasks:\n"
      "  build:\n"
      "    vars:\n"
      "      GREETING: overridden\n"
      "    cmds:\n"
      "      - echo {{.GREETING}}\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_EQ(GetNumberOfVarsInSeq(&task->vars), 1);
  VarNode* local = GetVarInSeqAt(&task->vars, 0);

  CommandNode* cmd = GetCommandInSeqAt(&task->cmds, 0);
  ASSERT_EQ(GetNumberOfRefsInString(&cmd->cmd), 1);
  RefNode* ref = GetStringRefAt(&cmd->cmd, 0);
  ASSERT_EQ(ref->to, (DocumentNode*)local);
}

TEST_F(TestParser, Test_Parse_TemplateRefWithPipeline) {
  static const char* kDoc =
      "vars:\n"
      "  VERSION: 1.0.0\n"
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - echo {{.VERSION | upper | default \"dev\"}}\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  CommandNode* cmd = GetCommandInSeqAt(&task->cmds, 0);
  ASSERT_EQ(GetNumberOfRefsInString(&cmd->cmd), 1);

  RefNode* ref = GetStringRefAt(&cmd->cmd, 0);
  ASSERT_EQ(std::string(ref->name), "VERSION");
  ASSERT_TRUE(ref->to != nullptr);
  ASSERT_EQ(ref->pipeline.len, 2);
  ASSERT_EQ(std::string(ref->pipeline.values[0].name), "upper");
  ASSERT_EQ(std::string(ref->pipeline.values[1].name), "default");
  ASSERT_EQ(std::string(ref->pipeline.values[1].args), "\"dev\"");
}

TEST_F(TestParser, Test_Parse_LeadingCommentAttachesToNextTask) {
  static const char* kDoc =
      "tasks:\n"
      "  # Runs the build.\n"
      "  # Second line.\n"
      "  build:\n"
      "    cmds:\n"
      "      - echo hi\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_EQ(GetNumberOfCommentsForNode((DocumentNode*)task), 1);
  CommentNode* comment = GetNodeCommentAt((DocumentNode*)task, 0);
  ASSERT_EQ(std::string(comment->value), "Runs the build.\nSecond line.");
}

TEST_F(TestParser, Test_Parse_CommentSeparatedByBlankLineFloats) {
  static const char* kDoc =
      "tasks:\n"
      "  # not attached, blank line follows\n"
      "\n"
      "  build:\n"
      "    cmds:\n"
      "      - echo hi\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_EQ(GetNumberOfCommentsForNode((DocumentNode*)task), 0);
  ASSERT_EQ(GetNumberOfCommentsInDocument(result.doc), 1);
}

TEST_F(TestParser, Test_Parse_TrailingCommentAttachesToCommand) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - go test ./...  # runs tests\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  CommandNode* cmd = GetCommandInSeqAt(&task->cmds, 0);
  ASSERT_TRUE(NodeHasTrailingComment((DocumentNode*)cmd));
  ASSERT_EQ(std::string(cmd->trailing_comment->value), "runs tests");
}

TEST_F(TestParser, Test_Parse_IncludeAliasAndTaskfilePath) {
  static const char* kDoc =
      "includes:\n"
      "  docker: ./docker/Taskfile.yml\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  ASSERT_EQ(GetNumberOfIncludesInDocument(result.doc), 1);
  IncludeNode* include = GetDocumentIncludeAt(result.doc, 0);
  ASSERT_EQ(std::string(include->taskfile), "./docker/Taskfile.yml");
  ASSERT_EQ(GetNumberOfIncludeAliases(include), 1);
  ASSERT_EQ(std::string(GetIncludeAliasAt(include, 0)->value), "docker");
}

TEST_F(TestParser, Test_Parse_TolerantOfUnterminatedQuote) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    desc: \"hello\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_EQ(GetNumberOfTasksInDocument(result.doc), 1);
}

TEST_F(TestParser, Test_Parse_DanglingDashMarksDocumentPartial) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      -\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_TRUE(IsPartialDocument(result.doc));
}

TEST_F(TestParser, Test_Parse_IncompleteKeyDoesNotCrashAndMarksPartial) {
  static const char* kDoc =
      "tasks:\n"
      "  buil";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_TRUE(IsPartialDocument(result.doc));
  ASSERT_EQ(GetNumberOfTasksInDocument(result.doc), 0);
}

TEST_F(TestParser, Test_Parse_WellFormedDocumentIsNotPartial) {
  static const char* kDoc =
      "version: '3'\n"
      "tasks:\n"
      "  build:\n"
      "    desc: Build the project\n"
      "    cmds:\n"
      "      - go build ./...\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_FALSE(IsPartialDocument(result.doc));
  ASSERT_EQ(std::string(GetDocumentVersion(result.doc)), "3");
}

TEST_F(TestParser, Test_Parse_UnquotedTaskNameWithColon) {
  static const char* kDoc =
      "tasks:\n"
      "  go:build:\n"
      "    cmds:\n"
      "      - go build ./...\n"
      "  go:test:\n"
      "    deps:\n"
      "      - go:build\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_FALSE(IsPartialDocument(result.doc));
  ASSERT_EQ(GetNumberOfTasksInDocument(result.doc), 2);

  TaskNode* build = GetDocumentTaskAt(result.doc, 0);
  ASSERT_EQ(std::string(build->name), "go:build");

  TaskNode* test_task = GetDocumentTaskAt(result.doc, 1);
  ASSERT_EQ(std::string(test_task->name), "go:test");
  ASSERT_EQ(GetNumberOfRefsInSeq(&test_task->deps), 1);
  ASSERT_EQ(GetRefInSeqAt(&test_task->deps, 0)->to, (DocumentNode*)build);
}

TEST_F(TestParser, Test_Parse_TaskNameWithColonUsesQuotedKey) {
  static const char* kDoc =
      "tasks:\n"
      "  \"docker:build\":\n"
      "    cmds:\n"
      "      - docker build .\n"
      "  test:\n"
      "    deps:\n"
      "      - \"docker:build\"\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_FALSE(IsPartialDocument(result.doc));
  ASSERT_EQ(GetNumberOfTasksInDocument(result.doc), 2);

  TaskNode* docker_build = GetDocumentTaskAt(result.doc, 0);
  ASSERT_EQ(std::string(docker_build->name), "docker:build");
  ASSERT_EQ(GetNumberOfCommandsInSeq(&docker_build->cmds), 1);

  TaskNode* test_task = GetDocumentTaskAt(result.doc, 1);
  ASSERT_EQ(std::string(test_task->name), "test");
  ASSERT_EQ(GetNumberOfRefsInSeq(&test_task->deps), 1);
  RefNode* dep = GetRefInSeqAt(&test_task->deps, 0);
  ASSERT_EQ(std::string(dep->name), "docker:build");
  ASSERT_EQ(dep->to, (DocumentNode*)docker_build);
}

TEST_F(TestParser, Test_Parse_TaskSingleCommandShorthandExpandsAndFlags) {
  static const char* kDoc =
      "tasks:\n"
      "  build: go build ./...\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_TRUE(IsTaskShortForm(task));
  ASSERT_EQ(GetNumberOfCommandsInSeq(&task->cmds), 1);
  ASSERT_EQ(std::string(GetCommandInSeqAt(&task->cmds, 0)->cmd.value), "go build ./...");
}

TEST_F(TestParser, Test_Parse_TaskBareArrayShorthandExpandsAndFlags) {
  static const char* kDoc =
      "tasks:\n"
      "  test:\n"
      "    - go test ./...\n"
      "    - echo done\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_TRUE(IsTaskShortForm(task));
  ASSERT_EQ(GetNumberOfCommandsInSeq(&task->cmds), 2);
  ASSERT_EQ(std::string(GetCommandInSeqAt(&task->cmds, 1)->cmd.value), "echo done");
}

TEST_F(TestParser, Test_Parse_TaskFullFormIsNotFlaggedShortForm) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    desc: Build the project\n"
      "    cmds:\n"
      "      - go build ./...\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_FALSE(IsTaskShortForm(task));
}

TEST_F(TestParser, Test_Visit_DocumentVisitsAllNestedNodes) {
  static const char* kDoc =
      "vars:\n"
      "  VERSION: 1.0.0\n"
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - echo {{.VERSION | upper}}\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  int counts[32] = {0};
  struct Ctx {
    int* counts;
  } ctx{counts};
  VisitDocument(
      result.doc,
      [](DocumentNode* node, void* data) {
        auto* c = static_cast<Ctx*>(data);
        if (node->kind < 32)
          c->counts[node->kind]++;
        return kVisitContinue;
      },
      nullptr, &ctx);

  ASSERT_EQ(counts[kTaskKind], 1);
  ASSERT_EQ(counts[kCommandKind], 1);
  ASSERT_EQ(counts[kStringKind], 1);
  ASSERT_EQ(counts[kRefKind], 1);
  ASSERT_EQ(counts[kPipelineExprKind], 1);
  ASSERT_EQ(counts[kVarKind], 1);
}

TEST_F(TestParser, Test_FindNodeAtPosition_FindsDeeplyNestedRef) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - echo {{.VERSION | upper}}\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  CommandNode* cmd = GetCommandInSeqAt(&task->cmds, 0);
  RefNode* ref = GetStringRefAt(&cmd->cmd, 0);

  DocumentNode* found = FindNodeAtPosition(result.doc, ref->start);
  ASSERT_EQ(found, (DocumentNode*)ref);

  DocumentNode* found_task = FindNodeAtPosition(result.doc, task->start);
  ASSERT_EQ(found_task, (DocumentNode*)task);

  DocumentNode* found_none = FindNodeAtPosition(result.doc, Pos{999, 1});
  ASSERT_EQ(found_none, nullptr);
}

TEST_F(TestParser, Test_Parse_EnvVarsAtDocumentAndTaskScope) {
  static const char* kDoc =
      "env:\n"
      "  LOG_LEVEL: debug\n"
      "tasks:\n"
      "  build:\n"
      "    env:\n"
      "      LOCAL: yes\n"
      "    cmds:\n"
      "      - echo $LOG_LEVEL and ${UNDECLARED}\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_EQ(GetNumberOfEnvVarsInDocument(result.doc), 1);

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_EQ(GetNumberOfVarsInSeq(&task->env), 1);

  CommandNode* cmd = GetCommandInSeqAt(&task->cmds, 0);
  ASSERT_EQ(GetNumberOfRefsInString(&cmd->cmd), 2);

  RefNode* log_level = GetStringRefAt(&cmd->cmd, 0);
  ASSERT_EQ(log_level->ref_kind, kEnvRefKind);
  ASSERT_TRUE(log_level->to != nullptr);

  RefNode* undeclared = GetStringRefAt(&cmd->cmd, 1);
  ASSERT_EQ(undeclared->ref_kind, kEnvRefKind);
  ASSERT_TRUE(undeclared->to == nullptr);
}

TEST_F(TestParser, Test_Parse_VarSecretAndExplicitValueSubkeys) {
  static const char* kDoc =
      "vars:\n"
      "  TOKEN:\n"
      "    value: abc123\n"
      "    secret: true\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  VarNode* token = GetDocumentVarAt(result.doc, 0);
  ASSERT_TRUE(IsVarSecret(token));
  ASSERT_EQ(token->var_kind, kScalarVarNodeKind);
  ASSERT_TRUE(IsStringNode(token->value));
  ASSERT_EQ(std::string(((StringNode*)token->value)->value), "abc123");
}

namespace {

bool PosLessOrEqual(const Pos a, const Pos b) {
  return a.row < b.row || (a.row == b.row && a.col <= b.col);
}

struct RangeCheckCtx {
  std::vector<DocumentNode*> stack;
  std::vector<std::string> failures;
};

std::string DescribeNode(DocumentNode* node) {
  std::ostringstream out;
  out << "kind=" << node->kind << " range=(" << node->start.row << "," << node->start.col << ")-(" << node->end.row
      << "," << node->end.col << ")";
  return out.str();
}

::testing::AssertionResult AssertRangesCoverChildren(Document* doc) {
  RangeCheckCtx ctx;
  VisitDocument(
      doc,
      [](DocumentNode* node, void* raw) {
        auto* c = static_cast<RangeCheckCtx*>(raw);
        if (!c->stack.empty()) {
          DocumentNode* parent = c->stack.back();
          if (!PosLessOrEqual(parent->start, node->start) || !PosLessOrEqual(node->end, parent->end)) {
            c->failures.push_back("child " + DescribeNode(node) + " is not contained within parent " +
                                  DescribeNode(parent));
          }
        }
        c->stack.push_back(node);
        return kVisitContinue;
      },
      [](DocumentNode*, void* raw) {
        static_cast<RangeCheckCtx*>(raw)->stack.pop_back();
        return kVisitContinue;
      },
      &ctx);

  if (ctx.failures.empty())
    return ::testing::AssertionSuccess();

  auto failure = ::testing::AssertionFailure();
  for (const auto& msg : ctx.failures)
    failure << msg << "\n";
  return failure;
}

}  // namespace

TEST_F(TestParser, Test_Ranges_TaskWithFullBodyCoversAllChildren) {
  static const char* kDoc =
      "vars:\n"
      "  VERSION: 1.0.0\n"
      "  CONFIG:\n"
      "    map:\n"
      "      database: postgres\n"
      "      port: 5432\n"
      "  COMMIT_HASH:\n"
      "    sh: git rev-parse HEAD\n"
      "  BUILD_VERSION:\n"
      "    ref: .VERSION\n"
      "env:\n"
      "  LOG_LEVEL: debug\n"
      "tasks:\n"
      "  build:\n"
      "    desc: Build the project\n"
      "    vars:\n"
      "      LOCAL: yes\n"
      "    env:\n"
      "      TASK_ENV: yes\n"
      "    deps: [gen]\n"
      "    cmds:\n"
      "      - echo {{.VERSION | upper}} and $LOG_LEVEL and ${UNSET}\n"
      "      - task: gen\n"
      "  gen:\n"
      "    cmds:\n"
      "      - echo gen\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_TRUE(AssertRangesCoverChildren(result.doc));
}

TEST_F(TestParser, Test_Ranges_ShortFormTasksCoverExpandedChildren) {
  static const char* kDoc =
      "tasks:\n"
      "  build: go build ./...\n"
      "  test:\n"
      "    - go test ./...\n"
      "    - echo done\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_TRUE(AssertRangesCoverChildren(result.doc));
}

TEST_F(TestParser, Test_Ranges_QuotedAndColonTaskNamesCoverChildren) {
  static const char* kDoc =
      "tasks:\n"
      "  \"docker:build\":\n"
      "    cmds:\n"
      "      - docker build .\n"
      "  go:test:\n"
      "    deps:\n"
      "      - \"docker:build\"\n"
      "      - go:build\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_TRUE(AssertRangesCoverChildren(result.doc));
}

TEST_F(TestParser, Test_Ranges_BlockScalarCommandCoversMultilineRefs) {
  static const char* kDoc =
      "vars:\n"
      "  GREETING: hello\n"
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - |\n"
      "        echo start\n"
      "        echo {{.GREETING}}\n"
      "        echo $PATH\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_TRUE(AssertRangesCoverChildren(result.doc));
}

TEST_F(TestParser, Test_Ranges_CommentsDoNotBreakContainment) {
  static const char* kDoc =
      "tasks:\n"
      "  # Runs the build.\n"
      "  # Second line.\n"
      "  build:\n"
      "    cmds:\n"
      "      - go build ./...  # compiles everything\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_TRUE(AssertRangesCoverChildren(result.doc));
}

TEST_F(TestParser, Test_Ranges_PartialDocumentsStillHaveValidRanges) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      -\n"
      "  test:\n"
      "    cmds:\n"
      "      - echo test\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_TRUE(AssertRangesCoverChildren(result.doc));
}

TEST_F(TestParser, Test_Parse_TaskScalarAndFlagFields) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    desc: Build it\n"
      "    summary: A longer summary\n"
      "    dir: ./src\n"
      "    label: build-label\n"
      "    prefix: bld\n"
      "    silent: true\n"
      "    internal: true\n"
      "    interactive: false\n"
      "    ignore_error: true\n"
      "    method: timestamp\n"
      "    run: once\n"
      "    cmds:\n"
      "      - go build ./...\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_EQ(std::string(task->desc), "Build it");
  ASSERT_EQ(std::string(task->summary), "A longer summary");
  ASSERT_EQ(std::string(task->dir), "./src");
  ASSERT_EQ(std::string(task->label), "build-label");
  ASSERT_EQ(std::string(task->prefix), "bld");
  ASSERT_TRUE(IsTaskSilent(task));
  ASSERT_TRUE(IsTaskInternal(task));
  ASSERT_FALSE(IsTaskInteractive(task));
  ASSERT_TRUE(IsTaskIgnoreError(task));
  ASSERT_EQ(task->method, kTimestampMethodKind);
  ASSERT_EQ(task->mode, kTaskRunOnceMode);
}

TEST_F(TestParser, Test_Parse_TaskStringSeqFieldsInlineAndBlock) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    aliases: [b, bld]\n"
      "    sources:\n"
      "      - ./src/**/*.go\n"
      "      - go.mod\n"
      "    generates:\n"
      "      - ./bin/app\n"
      "    platforms: [linux, darwin]\n"
      "    cmds:\n"
      "      - go build ./...\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_EQ(GetNumberOfAliasesInTask(task), 2);
  ASSERT_EQ(std::string(GetTaskAliasAt(task, 1)->value), "bld");
  ASSERT_EQ(GetNumberOfSourcesInTask(task), 2);
  ASSERT_EQ(std::string(GetTaskSourceAt(task, 0)->value), "./src/**/*.go");
  ASSERT_EQ(GetNumberOfGeneratesInTask(task), 1);
  ASSERT_EQ(GetNumberOfPlatformsInTask(task), 2);
}

TEST_F(TestParser, Test_Parse_TaskStatusAndPreconditions) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    status:\n"
      "      - test -f ./bin/app\n"
      "    preconditions:\n"
      "      - test -f go.mod\n"
      "      - sh: which go\n"
      "        msg: go must be installed\n"
      "    cmds:\n"
      "      - go build ./...\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_EQ(GetNumberOfCommandsInSeq(&task->status_cmds), 1);
  ASSERT_EQ(std::string(GetCommandInSeqAt(&task->status_cmds, 0)->cmd.value), "test -f ./bin/app");

  ASSERT_EQ(GetNumberOfPreconditionsInSeq(&task->preconditions), 2);
  PreconditionNode* shorthand = GetPreconditionInSeqAt(&task->preconditions, 0);
  ASSERT_TRUE(shorthand->command != nullptr);
  ASSERT_EQ(std::string(shorthand->command->cmd.value), "test -f go.mod");

  PreconditionNode* full = GetPreconditionInSeqAt(&task->preconditions, 1);
  ASSERT_TRUE(full->command != nullptr);
  ASSERT_EQ(std::string(full->command->cmd.value), "which go");
  ASSERT_TRUE(full->message != nullptr);
  ASSERT_EQ(std::string(full->message->value), "go must be installed");
}

TEST_F(TestParser, Test_Parse_TaskRequiresVars) {
  static const char* kDoc =
      "tasks:\n"
      "  deploy:\n"
      "    requires:\n"
      "      vars: [TARGET, REGION]\n"
      "    cmds:\n"
      "      - ./deploy.sh\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_EQ(GetNumberOfRequiredVarsInTask(task), 2);
  ASSERT_EQ(std::string(GetTaskRequiredVarAt(task, 0)->value), "TARGET");
  ASSERT_EQ(std::string(GetTaskRequiredVarAt(task, 1)->value), "REGION");
}

TEST_F(TestParser, Test_Parse_TaskCmdShorthandKey) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    cmd: go build ./...\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_TRUE(IsTaskShortForm(task));
  ASSERT_EQ(GetNumberOfCommandsInSeq(&task->cmds), 1);
  ASSERT_EQ(std::string(GetCommandInSeqAt(&task->cmds, 0)->cmd.value), "go build ./...");
}

TEST_F(TestParser, Test_Parse_RootDotenvList) {
  static const char* kDoc =
      "dotenv: ['.env', '.env.local']\n"
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - go build ./...\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_EQ(GetNumberOfDotenvsInDocument(result.doc), 2);
  ASSERT_EQ(std::string(GetDocumentDotenvAt(result.doc, 0)->value), ".env");
}

TEST_F(TestParser, Test_Ranges_AllNewSchemaFieldsCoverChildren) {
  static const char* kDoc =
      "dotenv: ['.env']\n"
      "tasks:\n"
      "  build:\n"
      "    desc: Build it\n"
      "    aliases: [b, bld]\n"
      "    sources:\n"
      "      - ./src/**/*.go\n"
      "    generates:\n"
      "      - ./bin/app\n"
      "    requires:\n"
      "      vars: [TARGET]\n"
      "    status:\n"
      "      - test -f ./bin/{{.TARGET}}\n"
      "    preconditions:\n"
      "      - test -f go.mod\n"
      "      - sh: which go\n"
      "        msg: go must be installed\n"
      "    cmds:\n"
      "      - go build ./...\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_TRUE(AssertRangesCoverChildren(result.doc));
}

TEST_F(TestParser, Test_Parse_ForAttributeListAndVarForms) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - for: sources\n"
      "        cmd: echo {{.ITEM}}\n"
      "      - for: [a, b, c]\n"
      "        cmd: echo item\n"
      "      - for:\n"
      "          var: MY_LIST\n"
      "          split: ','\n"
      "          as: THING\n"
      "        cmd: echo thing\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_EQ(GetNumberOfCommandsInSeq(&task->cmds), 3);

  ForNode* attr = GetCommandInSeqAt(&task->cmds, 0)->for_each;
  ASSERT_TRUE(attr != nullptr);
  ASSERT_EQ(attr->for_kind, kAttributeForKind);
  ASSERT_EQ(attr->attribute, kSourcesForAttribute);

  ForNode* list = GetCommandInSeqAt(&task->cmds, 1)->for_each;
  ASSERT_TRUE(list != nullptr);
  ASSERT_EQ(list->for_kind, kListForKind);
  ASSERT_EQ(GetNumberOfItemsInFor(list), 3);
  ASSERT_EQ(std::string(GetForItemAt(list, 2)->value), "c");

  ForNode* var_for = GetCommandInSeqAt(&task->cmds, 2)->for_each;
  ASSERT_TRUE(var_for != nullptr);
  ASSERT_EQ(var_for->for_kind, kVarForKind);
  ASSERT_EQ(std::string(var_for->var.name), "MY_LIST");
  ASSERT_TRUE(ForHasSplit(var_for));
  ASSERT_EQ(std::string(var_for->split), ",");
  ASSERT_EQ(std::string(var_for->as), "THING");
}

TEST_F(TestParser, Test_Parse_ForMatrixForm) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - for:\n"
      "          matrix:\n"
      "            OS: linux\n"
      "            ARCH: amd64\n"
      "        cmd: echo build\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  ForNode* loop = GetCommandInSeqAt(&GetDocumentTaskAt(result.doc, 0)->cmds, 0)->for_each;
  ASSERT_TRUE(loop != nullptr);
  ASSERT_EQ(loop->for_kind, kMatrixForKind);
  ASSERT_TRUE(loop->matrix != nullptr);
  ASSERT_EQ(GetNumberOfMapEntrysInSeq(&loop->matrix->entries), 2);
  ASSERT_EQ(std::string(GetMapEntryInSeqAt(&loop->matrix->entries, 0)->key), "OS");
}

TEST_F(TestParser, Test_Parse_DeferCommandForm) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - defer: echo cleanup\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  CommandNode* cmd = GetCommandInSeqAt(&GetDocumentTaskAt(result.doc, 0)->cmds, 0);
  ASSERT_TRUE(cmd->defer != nullptr);
  ASSERT_FALSE(IsDeferTaskCall(cmd->defer));
  ASSERT_TRUE(cmd->defer->cmd != nullptr);
  ASSERT_EQ(std::string(cmd->defer->cmd->value), "echo cleanup");
}

TEST_F(TestParser, Test_Parse_PerCommandIfPlatformsSetShopt) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - cmd: echo conditional\n"
      "        if: '{{.CI}}'\n"
      "        platforms: [linux, darwin]\n"
      "        set: [errexit, pipefail]\n"
      "        shopt: [globstar]\n"
      "        silent: true\n"
      "        ignore_error: true\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  CommandNode* cmd = GetCommandInSeqAt(&GetDocumentTaskAt(result.doc, 0)->cmds, 0);
  ASSERT_EQ(std::string(cmd->cmd.value), "echo conditional");
  ASSERT_TRUE(cmd->if_expr != nullptr);
  ASSERT_EQ(std::string(cmd->if_expr->value), "{{.CI}}");
  ASSERT_EQ(GetNumberOfStringsInSeq(&cmd->platforms), 2);
  ASSERT_TRUE(cmd->set != nullptr);
  ASSERT_TRUE(TestShellOpts(*cmd->set, kErrExitShellOpt));
  ASSERT_TRUE(TestShellOpts(*cmd->set, kPipeFailShellOpt));
  ASSERT_TRUE(cmd->shopt != nullptr);
  ASSERT_TRUE(TestShOpts(*cmd->shopt, kGlobStarShOpt));
  ASSERT_TRUE(cmd->silent);
  ASSERT_TRUE(cmd->ignore_error);
}

TEST_F(TestParser, Test_Parse_OutputScalarAndGroupForms) {
  static const char* kScalar = "output: prefixed\n";
  TaskfileParseResult scalar = ParseTaskfileDocumentStr(kScalar, strlen(kScalar));
  ASSERT_TRUE(IsParseOk(scalar));
  ASSERT_TRUE(GetDocumentOutput(scalar.doc) != nullptr);
  ASSERT_EQ(GetDocumentOutput(scalar.doc)->output_kind, kPrefixedOutputKind);

  static const char* kGroup =
      "output:\n"
      "  group:\n"
      "    begin: '::group::{{.TASK}}'\n"
      "    end: '::endgroup::'\n"
      "    error_only: true\n";
  TaskfileParseResult group = ParseTaskfileDocumentStr(kGroup, strlen(kGroup));
  ASSERT_TRUE(IsParseOk(group));
  OutputNode* out = GetDocumentOutput(group.doc);
  ASSERT_TRUE(out != nullptr);
  ASSERT_EQ(out->output_kind, kGroupOutputKind);
  ASSERT_EQ(std::string(out->begin), "::group::{{.TASK}}");
  ASSERT_EQ(std::string(out->end_template), "::endgroup::");
  ASSERT_TRUE(out->error_only);
}

TEST_F(TestParser, Test_Parse_IncludeObjectFormWithVars) {
  static const char* kDoc =
      "includes:\n"
      "  simple: ./simple/Taskfile.yml\n"
      "  docker:\n"
      "    taskfile: ./docker/Taskfile.yml\n"
      "    dir: ./docker\n"
      "    optional: true\n"
      "    flatten: true\n"
      "    aliases: [dk]\n"
      "    excludes: [clean]\n"
      "    vars:\n"
      "      REGISTRY: ghcr.io\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_EQ(GetNumberOfIncludesInDocument(result.doc), 2);

  IncludeNode* simple = GetDocumentIncludeAt(result.doc, 0);
  ASSERT_EQ(std::string(simple->taskfile), "./simple/Taskfile.yml");

  IncludeNode* docker = GetDocumentIncludeAt(result.doc, 1);
  ASSERT_EQ(std::string(docker->taskfile), "./docker/Taskfile.yml");
  ASSERT_EQ(std::string(docker->dir), "./docker");
  ASSERT_TRUE(docker->optional);
  ASSERT_TRUE(docker->flatten);
  ASSERT_EQ(GetNumberOfIncludeExcludes(docker), 1);
  ASSERT_EQ(GetNumberOfVarsInSeq(&docker->vars), 1);
  ASSERT_EQ(std::string(GetVarInSeqAt(&docker->vars, 0)->name), "REGISTRY");
}

TEST_F(TestParser, Test_Meta_ScalarStylesAreCaptured) {
  static const char* kDoc =
      "vars:\n"
      "  PLAIN: hello\n"
      "  SINGLE: 'hello'\n"
      "  DOUBLE: \"hello\"\n"
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - |\n"
      "        echo literal\n"
      "      - >\n"
      "        echo folded\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  ASSERT_EQ(GetDocumentVarAt(result.doc, 0)->value->style, kPlainScalarStyle);
  ASSERT_EQ(GetDocumentVarAt(result.doc, 1)->value->style, kSingleQuotedScalarStyle);
  ASSERT_EQ(GetDocumentVarAt(result.doc, 2)->value->style, kDoubleQuotedScalarStyle);

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_EQ(GetCommandInSeqAt(&task->cmds, 0)->cmd.style, kLiteralBlockScalarStyle);
  ASSERT_EQ(GetCommandInSeqAt(&task->cmds, 1)->cmd.style, kFoldedBlockScalarStyle);
  ASSERT_TRUE(IsBlockScalarStyle(GetCommandInSeqAt(&task->cmds, 0)->cmd.style));
  ASSERT_TRUE(IsQuotedScalarStyle(kSingleQuotedScalarStyle));
}

TEST_F(TestParser, Test_Meta_NewlineStyleLfAndCrLf) {
  static const char* kLf = "tasks:\n  build:\n    cmds:\n      - echo hi\n";
  TaskfileParseResult lf = ParseTaskfileDocumentStr(kLf, strlen(kLf));
  ASSERT_TRUE(IsParseOk(lf));
  ASSERT_EQ(GetDocumentNewlineStyle(lf.doc), kLfNewline);

  static const char* kCrLf = "tasks:\r\n  build:\r\n    cmds:\r\n      - echo hi\r\n";
  TaskfileParseResult crlf = ParseTaskfileDocumentStr(kCrLf, strlen(kCrLf));
  ASSERT_TRUE(IsParseOk(crlf));
  ASSERT_EQ(GetDocumentNewlineStyle(crlf.doc), kCrLfNewline);

  static const char* kMixed = "tasks:\n  build:\r\n    cmds:\n      - echo hi\n";
  TaskfileParseResult mixed = ParseTaskfileDocumentStr(kMixed, strlen(kMixed));
  ASSERT_TRUE(IsParseOk(mixed));
  ASSERT_EQ(GetDocumentNewlineStyle(mixed.doc), kMixedNewline);
}

TEST_F(TestParser, Test_Meta_IndentWidthDetection) {
  static const char* kTwo = "tasks:\n  build:\n    cmds:\n      - echo hi\n";
  TaskfileParseResult two = ParseTaskfileDocumentStr(kTwo, strlen(kTwo));
  ASSERT_TRUE(IsParseOk(two));
  ASSERT_EQ(GetDocumentIndentWidth(two.doc), 2);
  ASSERT_FALSE(DocumentUsesTabsForIndent(two.doc));

  static const char* kFour = "tasks:\n    build:\n        cmds:\n            - echo hi\n";
  TaskfileParseResult four = ParseTaskfileDocumentStr(kFour, strlen(kFour));
  ASSERT_TRUE(IsParseOk(four));
  ASSERT_EQ(GetDocumentIndentWidth(four.doc), 4);
}

TEST_F(TestParser, Test_Meta_TabIndentIsDetected) {
  static const char* kDoc = "tasks:\n\tbuild:\n\t\tcmds:\n\t\t\t- echo hi\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_TRUE(DocumentUsesTabsForIndent(result.doc));
}

TEST_F(TestParser, Test_Meta_BlankLinesRecordedAsLineNumbers) {
  static const char* kDoc =
      "version: '3'\n"
      "\n"
      "tasks:\n"
      "  build:\n"
      "\n"
      "    cmds:\n"
      "      - echo hi\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_EQ(GetNumberOfBlankLinesInDocument(result.doc), 2);
  ASSERT_TRUE(IsDocumentLineBlank(result.doc, 2));
  ASSERT_TRUE(IsDocumentLineBlank(result.doc, 5));
  ASSERT_FALSE(IsDocumentLineBlank(result.doc, 1));
}

TEST_F(TestParser, Test_Meta_NodeIndentAndDepthTracking) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    vars:\n"
      "      LOCAL: yes\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_EQ(task->indent, 2);

  VarNode* local = GetVarInSeqAt(&task->vars, 0);
  ASSERT_EQ(local->indent, 6);
  ASSERT_GT(local->depth, task->depth);
}

TEST_F(TestParser, Test_Meta_KeyAndValueSubRanges) {
  static const char* kDoc =
      "vars:\n"
      "  VERSION: 1.0.0\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  VarNode* var = GetDocumentVarAt(result.doc, 0);
  ASSERT_EQ(var->key_range.start.row, 2);
  ASSERT_EQ(var->key_range.start.col, 3);
  ASSERT_EQ(var->value_range.start.row, 2);
  ASSERT_GT(var->value_range.start.col, var->key_range.start.col);
}

TEST_F(TestParser, Test_Meta_TrailingCommentAttachesToItsOwnField) {
  static const char* kDoc =
      "tasks:\n"
      "  build:  # the build task\n"
      "    cmds:\n"
      "      - echo hi  # say hello\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  ASSERT_TRUE(NodeHasTrailingComment((DocumentNode*)task));
  ASSERT_EQ(std::string(task->trailing_comment->value), "the build task");

  CommandNode* cmd = GetCommandInSeqAt(&task->cmds, 0);
  ASSERT_TRUE(NodeHasTrailingComment((DocumentNode*)cmd));
  ASSERT_EQ(std::string(cmd->trailing_comment->value), "say hello");
  ASSERT_GT(cmd->trailing_ws, 0);
}

TEST_F(TestParser, Test_Meta_PerNodeIncompleteFlags) {
  static const char* kDoc =
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      -\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_TRUE(IsPartialDocument(result.doc));

  TaskNode* task = GetDocumentTaskAt(result.doc, 0);
  CommandNode* cmd = GetCommandInSeqAt(&task->cmds, 0);
  ASSERT_TRUE(IsNodeIncomplete((DocumentNode*)cmd));
  ASSERT_FALSE(IsNodeIncomplete((DocumentNode*)task));
}

TEST_F(TestParser, Test_Ranges_ForDeferAndIncludeNodesCoverChildren) {
  static const char* kDoc =
      "includes:\n"
      "  docker:\n"
      "    taskfile: ./docker/Taskfile.yml\n"
      "    aliases: [dk]\n"
      "    vars:\n"
      "      REGISTRY: ghcr.io\n"
      "tasks:\n"
      "  build:\n"
      "    cmds:\n"
      "      - for: [a, b]\n"
      "        cmd: echo {{.ITEM}}\n"
      "      - for:\n"
      "          matrix:\n"
      "            OS: linux\n"
      "        cmd: echo matrix\n"
      "      - defer: echo cleanup\n"
      "      - cmd: echo x\n"
      "        if: '{{.CI}}'\n"
      "        platforms: [linux]\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kDoc, strlen(kDoc));
  ASSERT_TRUE(IsParseOk(result));
  ASSERT_TRUE(AssertRangesCoverChildren(result.doc));
}

// NOLINTEND(cppcoreguidelines-pro-type-union-access)
