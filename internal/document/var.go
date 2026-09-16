package document

/*
#cgo pkg-config: taskfile-lsp-uninstalled

#include <stdlib.h>
#include "taskfile_parser.h"
#include "bridge.h"
*/
import "C"
import "unsafe"

type VarKind int

const (
	ScalarVar VarKind = C.kScalarVarNodeKind
	ShellVar  VarKind = C.kShellVarNodeKind
	MapVar    VarKind = C.kMapVarNodeKind
	RefVar    VarKind = C.kRefVarNodeKind
)

type Var struct {
	Node
	//	struct _VarNode {
	//	  DEFINE_DOCUMENT_NODE_FIELDS;
	//	      DocumentNode* value;
	//	      CommandNode* command;
	//	};
}

func (n *Var) toVarNode() *C.VarNode {
	if n == nil || n.handle == nil {
		return nil
	}

	return (*C.VarNode)(unsafe.Pointer(n.handle))
}

func (n *Var) Name() string {
	if n == nil || n.handle == nil {
		return ""
	}

	return toGoString(&n.toVarNode().name)
}

func (n *Var) Kind() VarKind {
	if n == nil || n.handle == nil {
		return ScalarVar
	}

	return VarKind(n.toVarNode().var_kind)
}

func (n *Var) IsScalar() bool {
	return n.Kind() == ScalarVar
}

func (n *Var) IsRef() bool {
	return n.Kind() == RefVar
}

func (n *Var) IsShell() bool {
	return n.Kind() == ShellVar
}

func (n *Var) IsMap() bool {
	return n.Kind() == MapVar
}

func (n *Var) IsSecret() bool {
	if n == nil || n.handle == nil {
		return false
	}

	return bool(n.toVarNode().secret)
}

func (n *Var) value() *C.DocumentNode {
	if n == nil || n.handle == nil {
		return nil
	}

	return (*C.DocumentNode)(n.toVarNode().value)
}

func (n *Var) Ref() Ref {
	if n.handle == nil {
		return Ref{}
	}

	return Ref{
		Node: Node{
			handle: (*C.DocumentNode)(unsafe.Pointer(n.toVarNode().ref)),
		},
	}
}
