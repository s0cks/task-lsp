package document

/*
#cgo pkg-config: taskfile-parser-uninstalled

#include <stdlib.h>
#include "taskfile_parser.h"
#include "bridge.h"

bool goVisitDocumentIncludes(uint64_t idx, IncludeNode* inc, void* data);
*/
import "C"
import (
	"runtime/cgo"
	"unsafe"
)

type Include struct {
	Node
}

func toInclude(node *C.DocumentNode) Include {
	return Include{Node: Node{handle: node}}
}

func (n *Include) toIncludeNode() *C.IncludeNode {
	if n == nil || n.handle == nil {
		return nil
	}

	return (*C.IncludeNode)(unsafe.Pointer(n.handle))
}

func (n *Include) Taskfile() string {
	if n == nil || n.handle == nil {
		return ""
	}

	return toGoString(&n.toIncludeNode().taskfile)
}

func (n *Include) Dir() string {
	if n == nil || n.handle == nil {
		return ""
	}

	return toGoString(&n.toIncludeNode().dir)
}

func (n *Include) IsFlatten() bool {
	if n == nil || n.handle == nil {
		return false
	}

	return bool(n.toIncludeNode().flatten)
}

func (n *Include) IsOptional() bool {
	if n == nil || n.handle == nil {
		return false
	}

	return bool(n.toIncludeNode().optional)
}

func (n *Include) IsInternal() bool {
	if n == nil || n.handle == nil {
		return false
	}

	return bool(n.toIncludeNode().internal)
}

func (n *Include) Namespace() string {
	if n == nil || n.handle == nil {
		return ""
	}

	if n.toIncludeNode().aliases.len == 0 {
		return ""
	}

	s := getStringInSeqAt(&n.toIncludeNode().aliases, 0)
	return toGoString(&s.value)
}

func (n *Include) GetNumberOfAliases() uint64 {
	if n == nil || n.handle == nil {
		return 0
	}

	return uint64(n.toIncludeNode().aliases.len)
}

func (n *Include) GetAliasAt(idx uint64) String {
	if n == nil || n.handle == nil {
		return String{}
	}

	s := getStringInSeqAt(&n.toIncludeNode().aliases, idx)
	return String{Node: Node{handle: (*C.DocumentNode)(unsafe.Pointer(s))}}
}

type IncludeVisitor func(idx uint64, inc Include) bool

func (doc *Document) VisitIncludes(vis IncludeVisitor) {
	if doc.Handle == nil {
		return
	}

	handle := cgo.NewHandle(vis)
	defer handle.Delete()

	C.VisitDocumentIncludes(
		doc.Handle,
		(C.IncludeVisitor)(unsafe.Pointer(C.goVisitDocumentIncludes)),
		unsafe.Pointer(&handle),
	)
}

//export goVisitDocumentIncludes
func goVisitDocumentIncludes(idx C.uint64_t, cInc *C.IncludeNode, data unsafe.Pointer) C.bool {
	handle := *(*cgo.Handle)(data)
	vis := handle.Value().(IncludeVisitor)
	return C.bool(vis(uint64(idx), toInclude((*C.DocumentNode)(unsafe.Pointer(cInc)))))
}
