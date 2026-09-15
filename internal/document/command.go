package document

/*
#cgo pkg-config: taskfile-lsp-uninstalled

#include <stdlib.h>
#include "taskfile_parser.h"

#include "bridge.h"
*/
import "C"

type Command struct {
	handle *C.CommandNode
}
