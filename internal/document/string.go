package document

/*
#cgo pkg-config: taskfile-lsp-uninstalled

#include <stdlib.h>
#include "taskfile_parser.h"
*/
import "C"
import (
	"unsafe"
)

type String struct {
	Node

	// RefSeq  refs
}

func (n *String) toStringNode() *C.StringNode {
	if n == nil || n.handle == nil {
		return nil
	}

	return (*C.StringNode)(unsafe.Pointer(n.handle))
}

func (n *String) Value() string {
	if n == nil || n.handle == nil {
		return ""
	}

	return toGoString(&n.toStringNode().value)
}

func (n *String) GetNumberOfRefs() uint64 {
	if n == nil || n.handle == nil {
		return 0
	}

	return uint64(n.toStringNode().refs.len)
}

func (n *String) GetRefAt(idx uint64) Ref {
	if n == nil || n.handle == nil {
		return Ref{}
	}

	return Ref{
		Node: Node{
			handle: (*C.DocumentNode)(unsafe.Pointer(getRefInSeqAt(&n.toStringNode().refs, idx))),
		},
	}
}
