#include "common.h"
#include "src/seq.h"
#include "taskfile_parser.h"

DEFINE_NODE_SEQ_HELPERS(Include, Aliases, String, StringNode, aliases)
DEFINE_NODE_SEQ_HELPERS(Include, Excludes, String, StringNode, excludes);
