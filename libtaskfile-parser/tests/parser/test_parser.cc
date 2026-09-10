#include <gtest/gtest.h>

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

TEST_F(TestParser, Test_Parse_MissingTasksField) {
  static const char* kTaskfileDocument =
      "---\n"
      "\n";
  TaskfileParseResult result = ParseTaskfileDocumentStr(kTaskfileDocument, strlen(kTaskfileDocument));
  ASSERT_TRUE(IsParseOk(result));
}

// NOLINTEND(cppcoreguidelines-pro-type-union-access)
