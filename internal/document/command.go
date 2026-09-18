package document

/*
#cgo pkg-config: taskfile-parser-uninstalled

#include <stdlib.h>
#include "taskfile_parser.h"

#include "bridge.h"
*/
import "C"
import "unsafe"

type Command struct {
	Node
}

func toCommand(node *C.CommandNode) Command {
	return Command{
		Node: Node{
			handle: (*C.DocumentNode)(unsafe.Pointer(node)),
		},
	}
}
