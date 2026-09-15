#ifndef TASKFILE_PARSER_COMMON_H
#define TASKFILE_PARSER_COMMON_H

#ifdef TASKFILE_PARSER_DEBUG
#include <assert.h>
#endif  // TASKFILE_PARSER_DEBUG

#ifndef ASSERT
#ifdef TASKFILE_PARSER_DEBUG
#define ASSERT(x) assert((x))
#else
#define ASSERT(x)
#endif  // TASKFILE_PARSER_DEBUG
#endif  // ASSERT

#ifndef ASSERT_EQ
#define ASSERT_EQ(a, b) ASSERT(a == b)
#endif  // ASSERT_EQ
#ifndef ASSERT_NE
#define ASSERT_NE(a, b) ASSERT(a != b)
#endif  // ASSERT_NE
#ifndef ASSERT_GT
#define ASSERT_GT(a, b) ASSERT(a > b)
#endif  // ASSERT_GT
#ifndef ASSERT_LT
#define ASSERT_LT(a, b) ASSERT(a < b)
#endif  // ASSERT_LT

#endif  // TASKFILE_PARSER_COMMON_H
