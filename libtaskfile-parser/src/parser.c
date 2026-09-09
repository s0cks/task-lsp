#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#define PARSE_ERROR(Format, ...)                                         \
  ({                                                                     \
    char* message = NULL;                                                \
    const size_t message_len = snprintf(NULL, 0, Format, ##__VA_ARGS__); \
    ASSERT_GT(message_len, 0);                                           \
    const size_t total_size = message_len + 1;                           \
    message = (char*)calloc(sizeof(char), total_size);                   \
    snprintf(message, total_size, Format, ##__VA_ARGS__);                \
    return (TaskfileParseResult){                                        \
        .success = false,                                                \
        .msg = message,                                                  \
    };                                                                   \
  })

TaskfileParseResult ParseTaskfileDocument(const char* data, const size_t data_len) {
  if (!data || data_len == 0)
    PARSE_ERROR("document is empty");

  PARSE_ERROR("not implemented");
}
