package rpc

import (
	"encoding/json"
	"fmt"
)

const (
	ParseError     = -32700
	InvalidRequest = -32600
	MethodNotFound = -32601
	InvalidParams  = -32602
	InternalError  = -32603
)

type Error struct {
	Code    int             `json:"code"`
	Message string          `json:"message"`
	Data    json.RawMessage `json:"data,omitempty"`
}

func (e *Error) Error() string { return fmt.Sprintf("rpc error %d: %s", e.Code, e.Message) }

func NewError(code int, message string) *Error { return &Error{Code: code, Message: message} }

func NewErrorf(code int, format string, args ...any) *Error {
	return NewError(code, fmt.Errorf(format, args...).Error())
}
