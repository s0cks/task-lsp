package document

/*
#cgo pkg-config: taskfile-parser-uninstalled

#include <stdlib.h>
#include "taskfile_parser.h"
*/
import "C"
import (
	"fmt"
	"unsafe"
)

type Document struct {
	Handle *C.Document
	source *C.char
}

func ParseDocumentString(value string) (*Document, error) {
	cValue := C.CString(value)

	result := C.ParseTaskfileDocumentStr(cValue, C.size_t(len(value)))
	if !result.success {
		goMessage := C.GoString(result.msg)
		C.FreeTaskfileParseResult(&result)
		C.free(unsafe.Pointer(cValue))
		return nil, fmt.Errorf("failed to parse Taskfile document: %s", goMessage)
	}

	return &Document{Handle: result.doc, source: cValue}, nil
}

func (doc *Document) Free() {
	if doc == nil || doc.Handle == nil {
		return
	}

	C.FreeDocument(doc.Handle)
	doc.Handle = nil

	if doc.source != nil {
		C.free(unsafe.Pointer(doc.source))
		doc.source = nil
	}
}

func (doc *Document) FindVar(name string) (Var, bool) {
	if doc == nil || doc.Handle == nil {
		return Var{}, false
	}

	v := C.FindDocumentVar(doc.Handle, toStrView(name))
	if v == nil {
		return Var{}, false
	}

	return Var{Node: Node{handle: (*C.DocumentNode)(unsafe.Pointer(v))}}, true
}

func (doc *Document) FindTaskAt(pos Pos) (Task, bool) {
	if doc == nil || doc.Handle == nil {
		return Task{}, false
	}

	var task Task
	var found bool
	doc.VisitTasks(func(idx uint64, t Task) bool {
		if t.Range().Contains(pos) {
			task = t
			found = true
		}

		return true
	})

	return task, found
}

func (doc *Document) FindTask(name string) (Task, bool) {
	if doc == nil || doc.Handle == nil {
		return Task{}, false
	}

	t := C.FindDocumentTask(doc.Handle, toStrView(name))
	if t == nil {
		return Task{}, false
	}

	return Task{Node: Node{handle: (*C.DocumentNode)(unsafe.Pointer(t))}}, true
}

//   V(Command)                           \
//   V(String)                            \
//   V(Bool)                              \
//   V(Number)                            \
//   V(Null)                              \
//   V(MapEntry)                          \
//   V(Map)                               \

// #define FOR_EACH_DOCUMENT_NODE_KIND(V) \
//   V(PipelineExpr)                      \
//   V(Set)                               \
