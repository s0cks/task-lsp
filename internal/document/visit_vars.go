package document

/*
#cgo pkg-config: taskfile-lsp-uninstalled

#include <stdlib.h>
#include "taskfile_parser.h"
#include "bridge.h"

bool goVisitDocumentVars(uint64_t idx, VarNode* v, void* data);
*/
import "C"
import (
	"runtime/cgo"
	"unsafe"
)

type VarVisitor func(idx uint64, v Var) bool

func (doc *Document) VisitVars(vis VarVisitor) {
	if doc.Handle == nil {
		return
	}

	handle := cgo.NewHandle(vis)
	defer handle.Delete()

	C.VisitDocumentVars(
		doc.Handle,
		(C.VarVisitor)(unsafe.Pointer(C.goVisitDocumentVars)),
		unsafe.Pointer(&handle),
	)
}

//export goVisitDocumentVars
func goVisitDocumentVars(idx C.uint64_t, cVar *C.VarNode, data unsafe.Pointer) C.bool {
	handle := *(*cgo.Handle)(data)
	vis := handle.Value().(VarVisitor)
	goVar := Var{Node: Node{handle: (*C.DocumentNode)(unsafe.Pointer(cVar))}}
	return C.bool(vis(uint64(idx), goVar))
}
