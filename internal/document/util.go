package document

/*
#cgo pkg-config: taskfile-parser-uninstalled

#include <stdlib.h>
#include "taskfile_parser.h"
#include "bridge.h"
*/
import "C"
import (
	"runtime/cgo"
	"unsafe"
)

type NodeStatus int

const (
	NodeOk         = 0
	NodeIncomplete = 1 << 0
	NodeError      = 1 << 1
)

func IsNodeOk(status NodeStatus) bool {
	return status == NodeOk
}

func IsNodeIncomplete(status NodeStatus) bool {
	return (status & NodeIncomplete) == NodeIncomplete
}

func IsNodeError(status NodeStatus) bool {
	return (status & NodeError) == NodeError
}

type Pos struct {
	Row int
	Col int
}

func toPos(cPos *C.Pos) Pos {
	return Pos{
		Row: int(cPos.row),
		Col: int(cPos.col),
	}
}

type Range struct {
	Start Pos
	End   Pos
}

func toRange(cRange *C.Range) Range {
	return Range{
		Start: toPos(&cRange.start),
		End:   toPos(&cRange.end),
	}
}

func toGoString(cStrView *C.StrView) string {
	return C.GoStringN(cStrView.start, C.int(cStrView.len))
}

type ScalarStyle int

const (
	NoScalarStyle           ScalarStyle = C.kNoneScalarStyle
	PlainScalarStyle        ScalarStyle = C.kPlainScalarStyle
	SingleQuoteScalarStyle  ScalarStyle = C.kSingleQuotedScalarStyle
	DoubleQuoteScalarStyle  ScalarStyle = C.kDoubleQuotedScalarStyle
	LiteralBlockScalarStyle ScalarStyle = C.kLiteralBlockScalarStyle
	FoldedBlockScalarStyle  ScalarStyle = C.kFoldedBlockScalarStyle
)

type Method int

const (
	NoMethod        Method = C.kNoneMethodKind
	ChecksumMethod  Method = C.kChecksumMethodKind
	TimestampMethod Method = C.kTimestampMethodKind
	DefaultMethod   Method = ChecksumMethod
)

func getCommentInSeqAt(seq *C.CommentSeq, idx uint64) *C.CommentNode {
	if seq == nil || seq.len == 0 {
		return nil
	}

	elemSize := unsafe.Sizeof(*seq.values)
	targetPtr := unsafe.Add(unsafe.Pointer(seq.values), uintptr(idx)*elemSize)
	return (*C.CommentNode)(targetPtr)
}

func getRefInSeqAt(seq *C.RefSeq, idx uint64) *C.RefNode {
	if seq == nil || seq.len == 0 {
		return nil
	}

	elemSize := unsafe.Sizeof(*seq.values)
	targetPtr := unsafe.Add(unsafe.Pointer(seq.values), uintptr(idx)*elemSize)
	return (*C.RefNode)(targetPtr)
}

func visitComments(node *C.DocumentNode, vis CommentVisitor) {
	handle := cgo.NewHandle(vis)
	defer handle.Delete()

	C.VisitNodeComments(
		node,
		(C.CommentVisitor)(unsafe.Pointer(C.goVisitComment)),
		unsafe.Pointer(&handle),
	)
}

func toStrView(s string) C.StrView {
	if len(s) == 0 {
		return C.StrView{}
	}

	return C.StrView{
		start: (*C.char)(unsafe.Pointer(unsafe.StringData(s))),
		len:   C.size_t(len(s)),
	}
}
