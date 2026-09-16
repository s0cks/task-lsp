package document

/*
#cgo pkg-config: taskfile-lsp-uninstalled

#include <stdlib.h>
#include "taskfile_parser.h"
#include "bridge.h"
*/
import "C"
import (
	"runtime/cgo"
	"unsafe"
)

type RefKind int

const (
	VarRef  RefKind = C.kVarRefKind
	EnvRef  RefKind = C.kEnvRefKind
	TaskRef RefKind = C.kTaskRefKind
)

type Ref struct {
	Node
	//TODO(@s0cks):
	// DEFINE_DOCUMENT_NODE_FIELDS;
	// PipelineExprSeq pipeline;
	// Range key_range;                  \
	// Range value_range;                \
	// DiagnosticSeq diagnostics;        \
}

func toRef(node *C.RefNode) Ref {
	return Ref{
		Node: Node{
			handle: (*C.DocumentNode)(unsafe.Pointer(node)),
		},
	}
}

type RefVisitor func(idx uint64, ref Ref) bool
type RefPredicate func(ref Ref) bool

func (n *Ref) toRefNode() *C.RefNode {
	if n == nil || n.handle == nil {
		return nil
	}

	return (*C.RefNode)(unsafe.Pointer(n.handle))
}

func (n *Ref) Name() string {
	if n.handle == nil {
		return ""
	}

	return toGoString(&n.toRefNode().name)
}

func (n *Ref) Kind() RefKind {
	if n.handle == nil {
		return VarRef
	}

	return RefKind(n.toRefNode().ref_kind)
}

func (n *Ref) from() *C.DocumentNode {
	if n.handle == nil {
		return nil
	}

	return n.toRefNode().from
}

func (n *Ref) to() *C.DocumentNode {
	if n.handle == nil {
		return nil
	}

	return n.toRefNode().to
}

func (n *Ref) Style() ScalarStyle {
	if n.handle == nil {
		return NoScalarStyle
	}

	return ScalarStyle(n.handle.style)
}

func (n *Ref) Status() NodeStatus {
	if n == nil {
		return NodeError
	}

	return NodeStatus(n.handle.status)
}

func (n *Ref) IsOk() bool {
	return n.Status() == NodeOk
}

func (n *Ref) IsIncomplete() bool {
	return n.Status() == NodeIncomplete
}

func (n *Ref) IsError() bool {
	return (n.Status() & NodeError) == NodeError
}

//export goVisitRef
func goVisitRef(idx C.uint64_t, cRef *C.RefNode, data unsafe.Pointer) C.bool {
	handle := *(*cgo.Handle)(data)
	vis := handle.Value().(RefVisitor)
	keepGoing := vis(uint64(idx), toRef(cRef))
	return C.bool(keepGoing)
}
