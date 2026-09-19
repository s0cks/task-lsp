package document

/*
#cgo pkg-config: taskfile-parser-uninstalled

#include <stdlib.h>
#include "taskfile_parser.h"

#include "bridge.h"
*/
import "C"
import "unsafe"

type Output struct {
	Node
	// OutputKind output_kind;
}

func (n *Output) toOutputNode() *C.OutputNode {
	if n == nil || n.handle == nil {
		return nil
	}

	return (*C.OutputNode)(unsafe.Pointer(n.handle))
}

func (n *Output) IsErrorOnly() bool {
	if n == nil || n.handle == nil {
		return false
	}

	return bool(n.toOutputNode().error_only)
}

func (n *Output) Begin() string {
	if n == nil || n.handle == nil {
		return ""
	}

	return toGoString(&n.toOutputNode().begin)
}

func (n *Output) EndTemplate() string {
	if n == nil || n.handle == nil {
		return ""
	}

	return toGoString(&n.toOutputNode().end_template)
}
