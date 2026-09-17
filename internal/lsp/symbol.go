package lsp

type SymbolKind int

const (
	FileSymbol          SymbolKind = 1
	ModuleSymbol        SymbolKind = 2
	NamespaceSymbol     SymbolKind = 3
	PackageSymbol       SymbolKind = 4
	ClassSymbol         SymbolKind = 5
	MethodSymbol        SymbolKind = 6
	PropertySymbol      SymbolKind = 7
	FieldSymbol         SymbolKind = 8
	ConstructorSymbol   SymbolKind = 9
	EnumSymbol          SymbolKind = 10
	InterfaceSymbol     SymbolKind = 11
	FunctionSymbol      SymbolKind = 12
	VariableSymbol      SymbolKind = 13
	ConstantSymbol      SymbolKind = 14
	StringSymbol        SymbolKind = 15
	NumberSymbol        SymbolKind = 16
	BooleanSymbol       SymbolKind = 17
	ArraySymbol         SymbolKind = 18
	ObjectSymbol        SymbolKind = 19
	KeySymbol           SymbolKind = 20
	NullSymbol          SymbolKind = 21
	EnumMemberSymbol    SymbolKind = 22
	StructSymbol        SymbolKind = 23
	EventSymbol         SymbolKind = 24
	OperatorSymbol      SymbolKind = 25
	TypeParameterSymbol SymbolKind = 26
)

type Symbol struct {
	Name           string     `json:"name"`
	Detail         string     `json:"detail"`
	Kind           SymbolKind `json:"kind"`
	Range          Range      `json:"range"`
	SelectionRange Range      `json:"selectionRange"`
}
