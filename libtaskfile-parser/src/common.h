#ifndef TASKFILE_PARSER_COMMON_H
#define TASKFILE_PARSER_COMMON_H

#ifdef TASKFILE_PARSER_DEBUG

#include <assert.h>

#ifndef ASSERT

#define ASSERT(x)       assert((x))
#define ASSERT_EQ(a, b) ASSERT(a == b)
#define ASSERT_NE(a, b) ASSERT(a != b)
#define ASSERT_GT(a, b) ASSERT(a > b)
#define ASSERT_LT(a, b) ASSERT(a < b)

#else

#define ASSERT(x)
#define ASSERT_EQ(a, b)
#define ASSERT_NE(a, b)
#define ASSERT_GT(a, b)
#define ASSERT_LT(a, b)

#endif  // TASKFILE_PARSER_DEBUG

#endif  // ASSERT

#endif  // TASKFILE_PARSER_COMMON_H
