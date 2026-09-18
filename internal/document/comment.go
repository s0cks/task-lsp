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

type Comment struct {
	Node
}

func (n *Comment) toCommentNode() *C.CommentNode {
	if n == nil || n.handle == nil {
		return nil
	}

	return (*C.CommentNode)(unsafe.Pointer(n.handle))
}

func toComment(node *C.DocumentNode) Comment {
	return Comment{
		Node: Node{
			handle: node,
		},
	}
}

type CommentPredicate func(c Comment) bool
type CommentVisitor func(idx uint64, c Comment) bool

func (n *Comment) Style() ScalarStyle {
	if n.handle == nil {
		return NoScalarStyle
	}

	return ScalarStyle(n.handle.style)
}

func (n *Comment) Status() NodeStatus {
	if n == nil {
		return NodeError
	}

	return NodeStatus(n.handle.status)
}

func (n *Comment) IsOk() bool {
	return n.Status() == NodeOk
}

func (n *Comment) IsIncomplete() bool {
	return n.Status() == NodeIncomplete
}

func (n *Comment) IsError() bool {
	return (n.Status() & NodeError) == NodeError
}

func (n *Comment) Value() string {
	if n.handle == nil {
		return ""
	}

	return toGoString(&n.toCommentNode().value)
}

//export goVisitComment
func goVisitComment(idx C.uint64_t, cNode *C.CommentNode, data unsafe.Pointer) C.bool {
	handle := *(*cgo.Handle)(data)
	vis := handle.Value().(CommentVisitor)
	return C.bool(vis(uint64(idx), toComment((*C.DocumentNode)(unsafe.Pointer(cNode)))))
}
