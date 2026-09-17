package document

/*
#cgo pkg-config: taskfile-parser-uninstalled

#include <stdlib.h>
#include "taskfile_parser.h"
*/
import "C"
import (
	"runtime/cgo"
	"unsafe"
)

type String struct {
	Node

	// RefSeq  refs
}

type StringVisitor func(idx uint64, s String) bool
type StringPredicate func(idx uint64, s String) bool

func toString(node *C.DocumentNode) String {
	return String{
		Node: Node{
			handle: node,
		},
	}
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

//export goVisitString
func goVisitString(idx C.uint64_t, cStr *C.StringNode, data unsafe.Pointer) C.bool {
	handle := *(*cgo.Handle)(data)
	vis := handle.Value().(StringVisitor)

	goStr := toString((*C.DocumentNode)(unsafe.Pointer(cStr)))
	return C.bool(vis(uint64(idx), goStr))
}
