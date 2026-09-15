#include "taskfile_parser.h"

const char* ScalarStyleGetName(const ScalarStyle style) {
  switch (style) {
#define DEFINE_CASE(Name)    \
  case k##Name##ScalarStyle: \
    return #Name;
    FOR_EACH_SCALAR_STYLE(DEFINE_CASE)
#undef DEFINE_CASE
    default:
      return "Unknown";
  }
}

const char* NewlineStyleGetName(const NewlineStyle style) {
  switch (style) {
#define DEFINE_CASE(Name) \
  case k##Name##Newline:  \
    return #Name;
    FOR_EACH_NEWLINE_STYLE(DEFINE_CASE)
#undef DEFINE_CASE
    default:
      return "Unknown";
  }
}
