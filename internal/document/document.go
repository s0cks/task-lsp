package document

/*
#cgo pkg-config: taskfile-lsp-uninstalled

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
}

func ParseDocumentString(value string) (*Document, error) {
	cValue := C.CString(value)
	defer C.free(unsafe.Pointer(cValue))

	result := C.ParseTaskfileDocumentStr(cValue, C.size_t(len(value)))
	if !result.success {
		goMessage := C.GoString(result.msg)
		C.free(unsafe.Pointer(result.msg))
		return nil, fmt.Errorf("failed to parse Taskfile document: %s", goMessage)
	}

	return &Document{Handle: result.doc}, nil
}

func (doc *Document) Free() {
	if doc.Handle != nil {
		C.FreeDocument(doc.Handle)
	}
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
