package document

/*
#cgo pkg-config: taskfile-lsp-uninstalled

#include <stdlib.h>
#include "taskfile_parser.h"
*/
import "C"
import "unsafe"

type Node struct {
	handle *C.DocumentNode
}

func (n *Node) Style() ScalarStyle {
	if n == nil || n.handle == nil {
		return NoScalarStyle
	}

	return ScalarStyle(n.handle.style)
}

func (n *Node) Status() NodeStatus {
	if n == nil || n.handle == nil {
		return NodeError
	}

	return NodeStatus(n.handle.status)
}

func (n *Node) IsOk() bool {
	return n.Status() == NodeOk
}

func (n *Node) IsIncomplete() bool {
	return n.Status() == NodeIncomplete
}

func (n *Node) IsError() bool {
	return (n.Status() & NodeError) == NodeError
}

func (n *Node) Indent() int {
	if n.handle == nil {
		return 0
	}

	return int(n.handle.indent)
}

func (n *Node) Depth() int {
	if n.handle == nil {
		return -1
	}

	return int(n.handle.depth)
}

func (n *Node) LeadingWhitespace() int {
	if n.handle == nil {
		return -1
	}

	return int(n.handle.leading_ws)
}

func (n *Node) TrailingWhitespace() int {
	if n.handle == nil {
		return -1
	}

	return int(n.handle.trailing_ws)
}

func (n *Node) HasTrailingComment() bool {
	if n == nil || n.handle == nil {
		return false
	}

	return n.handle.trailing_comment != nil
}

func (n *Node) GetTrailingComment() Comment {
	if n == nil || n.handle == nil {
		return Comment{}
	}

	return Comment{
		Node: Node{
			handle: (*C.DocumentNode)(unsafe.Pointer(n.handle.trailing_comment)),
		},
	}
}

func (n *Node) GetNumberOfComments() uint64 {
	if n == nil || n.handle == nil {
		return 0
	}

	return uint64(n.handle.comments.len)
}

func (n *Node) GetCommentAt(idx uint64) Comment {
	if n.handle == nil {
		return Comment{}
	}

	return Comment{
		Node: Node{
			handle: (*C.DocumentNode)(unsafe.Pointer(getCommentInSeqAt(&n.handle.comments, idx))),
		},
	}
}

func (n *Node) VisitComments(vis CommentVisitor) {
	if n.handle == nil {
		return
	}

	visitComments((*C.DocumentNode)(n.handle), vis)
}

func (n *Node) Start() Pos {
	if n == nil || n.handle == nil {
		return Pos{}
	}

	return toPos(&n.handle.start)
}

func (n *Node) End() Pos {
	if n == nil || n.handle == nil {
		return Pos{}
	}

	return toPos(&n.handle.end)
}

func (n *Node) Range() Range {
	if n.handle == nil {
		return Range{}
	}

	return Range{
		Start: n.Start(),
		End:   n.End(),
	}
}
