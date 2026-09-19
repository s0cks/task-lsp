package document

/*
#cgo pkg-config: taskfile-parser-uninstalled

#include <stdlib.h>
#include "taskfile_parser.h"

#include "bridge.h"
*/
import "C"
import "unsafe"

type Defer struct {
	Node
}

func (n *Defer) toDeferNode() *C.DeferNode {
	if n == nil || n.handle == nil {
		return nil
	}

	return (*C.DeferNode)(unsafe.Pointer(n.handle))
}

func toDefer(node *C.DocumentNode) Defer {
	return Defer{Node: Node{handle: node}}
}

func (n *Defer) IsTaskCall() bool {
	if n == nil || n.handle == nil {
		return false
	}

	return bool(n.toDeferNode().is_task_call)
}

func (n *Defer) TaskCall() Ref {
	if n == nil || n.handle == nil {
		return Ref{}
	}

	return toRef(&n.toDeferNode().task_call)
}

func (n *Defer) Command() String {
	if n == nil || n.handle == nil {
		return String{}
	}

	return toString((*C.DocumentNode)(unsafe.Pointer(n.toDeferNode().cmd)))
}
