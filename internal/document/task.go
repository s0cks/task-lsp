package document

/*
#cgo pkg-config: taskfile-lsp-uninstalled

#include <stdlib.h>
#include "taskfile_parser.h"
#include "bridge.h"

bool goVisitDocumentTasks(uint64_t idx, TaskNode* task, void* data);
*/
import "C"
import (
	"runtime/cgo"
	"unsafe"
)

type TaskRunMode int

const (
	RunAlways      TaskRunMode = C.kTaskRunAlwaysMode
	RunOnce        TaskRunMode = C.kTaskRunOnceMode
	RunWhenChanged TaskRunMode = C.kTaskRunWhenChangedMode
	RunDefault     TaskRunMode = C.kDefaultTaskRunMode
)

type TaskFlags int

const (
	NoTaskFlags     TaskFlags = 0
	SilentFlag      TaskFlags = C.kSilentFlag
	GitignoreFlag   TaskFlags = C.kGitignoreFlag
	InternalFlag    TaskFlags = C.kInternalFlag
	InteractiveFlag TaskFlags = C.kInteractiveFlag
	PrefixedFlag    TaskFlags = C.kPrefixedFlag
	IgnoreErrorFlag TaskFlags = C.kIgnoreErrorFlag
	WatchFlag       TaskFlags = C.kWatchFlag
)

type Task struct {
	Node
	// DiagnosticSeq diagnostics;        \
	// StrView interval;
	// ShellOpts* set;
	// ShOpts* shopt;
	// StrViewSeq reqs;
	// CommandSeq cmds;
	// RefSeq deps;
	// VarSeq vars;
	// VarSeq env;
	// CommandSeq status_cmds;
	// PreconditionSeq preconditions;
	// StringSeq aliases;
	// StringSeq prompts;
	// StringSeq sources;
	// StringSeq generates;
	// StringSeq requires_vars;
	// StringSeq dotenvs;
	// StringSeq platforms;
}

func (n *Task) toTaskNode() *C.TaskNode {
	if n == nil || n.handle == nil {
		return nil
	}

	return (*C.TaskNode)(unsafe.Pointer(n.handle))
}

func (n *Task) Mode() TaskRunMode {
	if n == nil || n.handle == nil {
		return RunDefault
	}

	return TaskRunMode(n.toTaskNode().mode)
}

func (n *Task) Flags() TaskFlags {
	if n == nil || n.handle == nil {
		return NoTaskFlags
	}

	return TaskFlags(n.toTaskNode().flags)
}

func (n *Task) IsSilent() bool {
	return (n.Flags() & SilentFlag) == SilentFlag
}

func (n *Task) IsGitignore() bool {
	return (n.Flags() & GitignoreFlag) == GitignoreFlag
}

func (n *Task) IsInternal() bool {
	return (n.Flags() & InternalFlag) == InternalFlag
}

func (n *Task) IsInteractive() bool {
	return (n.Flags() & InteractiveFlag) == InteractiveFlag
}

func (n *Task) IsPrefixed() bool {
	return (n.Flags() & PrefixedFlag) == PrefixedFlag
}

func (n *Task) IsIgnoreError() bool {
	return (n.Flags() & IgnoreErrorFlag) == IgnoreErrorFlag
}

func (n *Task) IsWatch() bool {
	return (n.Flags() & WatchFlag) == WatchFlag
}

func (n *Task) Method() Method {
	if n == nil || n.handle == nil {
		return NoMethod
	}

	return Method(n.toTaskNode().method)
}

func (n *Task) IsShortForm() bool {
	if n == nil || n.handle == nil {
		return false
	}

	return bool(n.toTaskNode().is_short_form)
}

func (n *Task) Prefix() string {
	if n == nil || n.handle == nil {
		return ""
	}

	return toGoString(&n.toTaskNode().prefix)
}

func (n *Task) Label() string {
	if n == nil || n.handle == nil {
		return ""
	}

	return toGoString(&n.toTaskNode().label)
}

func (n *Task) Name() string {
	if n == nil || n.handle == nil {
		return ""
	}

	return toGoString(&n.toTaskNode().name)
}

func (n *Task) Desc() string {
	if n.handle == nil {
		return ""
	}

	return toGoString(&n.toTaskNode().desc)
}

func (n *Task) Dir() string {
	if n.handle == nil {
		return ""
	}

	return toGoString(&n.toTaskNode().dir)
}

func (n *Task) Summary() string {
	if n.handle == nil {
		return ""
	}

	return toGoString(&n.toTaskNode().summary)
}

type TaskVisitor func(idx uint64, task Task) bool
type TaskPredicate func(task Task) bool

func (doc *Document) VisitTasks(vis TaskVisitor) {
	if doc.Handle == nil {
		return
	}

	handle := cgo.NewHandle(vis)
	defer handle.Delete()

	C.VisitDocumentTasks(
		doc.Handle,
		(C.TaskVisitor)(unsafe.Pointer(C.goVisitDocumentTasks)),
		unsafe.Pointer(&handle),
	)
}

//export goVisitDocumentTasks
func goVisitDocumentTasks(idx C.uint64_t, cTask *C.TaskNode, data unsafe.Pointer) C.bool {
	handle := *(*cgo.Handle)(data)
	vis := handle.Value().(TaskVisitor)
	goTask := Task{
		Node: Node{
			handle: (*C.DocumentNode)(unsafe.Pointer(cTask)),
		},
	}
	keepGoing := vis(uint64(idx), goTask)
	return C.bool(keepGoing)
}
