package taskfile

type Severity int

const (
	SeverityError Severity = iota + 1
	SeverityWarning
	SeverityInformation
	SeverityHint
)

const (
	CodeDuplicateTask  = "duplicate-task"
	CodeUndefinedTask  = "undefined-task"
	CodeSelfDependency = "self-dependency"
)

type Position struct {
	Line      int
	Character int
}

type Range struct {
	Start Position
	End   Position
}

type Diagnostic struct {
	Range    Range
	Severity Severity
	Code     string
	Message  string
}

type RefKind int

const (
	RefDep RefKind = iota
	RefCall
)

type Ref struct {
	Name  string
	Owner string
	Kind  RefKind
	Range Range
}

type Task struct {
	Name      string
	Desc      string
	NameRange Range
	DefLine   int
	EndLine   int
}

type LineContext struct {
	Kind     string
	TaskName string
}

type File struct {
	Tasks      map[string]*Task
	Order      []string
	Refs       []Ref
	Includes   map[string]string
	TaskIndent int
	BodyIndent int
	TasksLine  int

	lineContext []LineContext
}

func (f *File) ContextAt(pos Position) LineContext {
	if pos.Line < 0 || pos.Line >= len(f.lineContext) {
		return LineContext{Kind: "root"}
	}
	return f.lineContext[pos.Line]
}

func inRange(r Range, pos Position) bool {
	if pos.Line != r.Start.Line || pos.Line != r.End.Line {
		return false
	}
	return pos.Character >= r.Start.Character && pos.Character <= r.End.Character
}

func (f *File) NameOrRefAt(pos Position) (string, bool) {
	for _, ref := range f.Refs {
		if inRange(ref.Range, pos) {
			return ref.Name, true
		}
	}
	for _, name := range f.Order {
		if t, ok := f.Tasks[name]; ok && inRange(t.NameRange, pos) {
			return t.Name, true
		}
	}
	return "", false
}

func (f *File) TaskAt(pos Position) (*Task, bool) {
	for _, name := range f.Order {
		t := f.Tasks[name]
		if pos.Line >= t.DefLine && pos.Line <= t.EndLine {
			return t, true
		}
	}
	return nil, false
}
