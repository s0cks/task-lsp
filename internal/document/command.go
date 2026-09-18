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

	// StringNode cmd;
	// ShellOpts* set;
	// ShOpts* shopt;
	// bool silent;
	// bool ignore_error;
	// StringSeq platforms;
	// StrView timeout;  // TODO(@s0cks): convert to Time expr node
	// RefNode task_call;
	// StringNode* if_expr;
	// ForNode* for_each;
	// DeferNode* defer;
}

func toCommand(node *C.CommandNode) Command {
	return Command{
		Node: Node{
			handle: (*C.DocumentNode)(unsafe.Pointer(node)),
		},
	}
}

func (n *Command) toCommandNode() *C.CommandNode {
	if n == nil || n.handle == nil {
		return nil
	}

	return (*C.CommandNode)(unsafe.Pointer(n.handle))
}
