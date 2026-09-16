#ifndef TASKFILE_PARSER_BRIDGE_H
#define TASKFILE_PARSER_BRIDGE_H

#include "taskfile_parser.h"

bool goVisitComment(uint64_t, CommentNode*, void*);
bool goVisitDocumentVars(uint64_t, VarNode*, void*);
bool goVisitRef(uint64_t, RefNode*, void*);

#endif  // TASKFILE_PARSER_BRIDGE_H
