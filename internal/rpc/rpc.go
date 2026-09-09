package rpc

import (
	"bufio"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"strconv"
	"strings"
	"sync"
)

const Version = "2.0"

const (
	ParseError     = -32700
	InvalidRequest = -32600
	MethodNotFound = -32601
	InvalidParams  = -32602
	InternalError  = -32603
)

type ID struct {
	value any
}

func NewIntID(n int64) ID { return ID{value: n} }

func NewStringID(s string) ID { return ID{value: s} }

func (id ID) MarshalJSON() ([]byte, error) {
	if id.value == nil {
		return []byte("null"), nil
	}

	return json.Marshal(id.value)
}

func (id *ID) UnmarshalJSON(data []byte) error {
	var raw any
	if err := json.Unmarshal(data, &raw); err != nil {
		return err
	}

	switch v := raw.(type) {
	case float64:
		id.value = int64(v)

	case string:
		id.value = v

	case nil:
		id.value = nil

	default:
		return fmt.Errorf("rpc: unsupported id type %T", raw)
	}

	return nil
}

func (id ID) String() string {
	switch v := id.value.(type) {
	case int64:
		return strconv.FormatInt(v, 10)

	case string:
		return v

	default:
		return ""

	}
}

type Request struct {
	JSONRPC string          `json:"jsonrpc"`
	ID      *ID             `json:"id,omitempty"`
	Method  string          `json:"method"`
	Params  json.RawMessage `json:"params,omitempty"`
}

func (r *Request) IsNotification() bool { return r.ID == nil }

type Error struct {
	Code    int             `json:"code"`
	Message string          `json:"message"`
	Data    json.RawMessage `json:"data,omitempty"`
}

func (e *Error) Error() string { return fmt.Sprintf("rpc error %d: %s", e.Code, e.Message) }

func NewError(code int, message string) *Error { return &Error{Code: code, Message: message} }

type Response struct {
	JSONRPC string          `json:"jsonrpc"`
	ID      *ID             `json:"id"`
	Result  json.RawMessage `json:"result,omitempty"`
	Error   *Error          `json:"error,omitempty"`
}

type envelope struct {
	JSONRPC string          `json:"jsonrpc"`
	ID      *ID             `json:"id,omitempty"`
	Method  string          `json:"method,omitempty"`
	Params  json.RawMessage `json:"params,omitempty"`
	Result  json.RawMessage `json:"result,omitempty"`
	Error   *Error          `json:"error,omitempty"`
}

func readMessage(r *bufio.Reader) ([]byte, error) {
	contentLength := -1
	for {
		line, err := r.ReadString('\n')
		if err != nil {
			return nil, err
		}

		line = strings.TrimRight(line, "\r\n")
		if line == "" {
			break
		}

		parts := strings.SplitN(line, ":", 2)
		if len(parts) != 2 {
			continue
		}

		name := strings.TrimSpace(parts[0])
		value := strings.TrimSpace(parts[1])
		if strings.EqualFold(name, "Content-Length") {
			n, err := strconv.Atoi(value)
			if err != nil {
				return nil, fmt.Errorf("rpc: invalid Content-Length %q: %w", value, err)
			}

			contentLength = n
		}
	}

	if contentLength < 0 {
		return nil, fmt.Errorf("rpc: message missing Content-Length header")
	}

	buf := make([]byte, contentLength)
	if _, err := io.ReadFull(r, buf); err != nil {
		return nil, err
	}

	return buf, nil
}

func writeMessage(w io.Writer, payload []byte) error {
	if _, err := fmt.Fprintf(w, "Content-Length: %d\r\n\r\n", len(payload)); err != nil {
		return err
	}

	_, err := w.Write(payload)
	return err
}

type RequestFunc func(ctx context.Context, conn *Conn, params json.RawMessage) (any, *Error)
type NotificationFunc func(ctx context.Context, conn *Conn, params json.RawMessage)

type Conn struct {
	reader  *bufio.Reader
	writer  io.Writer
	writeMu sync.Mutex

	handlerMu    sync.RWMutex
	requestFuncs map[string]RequestFunc
	notifyFuncs  map[string]NotificationFunc

	pendingMu sync.Mutex
	pending   map[string]chan *Response
	nextID    int64
}

func NewConn(r io.Reader, w io.Writer) *Conn {
	return &Conn{
		reader:       bufio.NewReader(r),
		writer:       w,
		requestFuncs: make(map[string]RequestFunc),
		notifyFuncs:  make(map[string]NotificationFunc),
		pending:      make(map[string]chan *Response),
	}
}

func (c *Conn) HandleRequest(method string, fn RequestFunc) {
	c.handlerMu.Lock()
	defer c.handlerMu.Unlock()
	c.requestFuncs[method] = fn
}

func (c *Conn) HandleNotification(method string, fn NotificationFunc) {
	c.handlerMu.Lock()
	defer c.handlerMu.Unlock()
	c.notifyFuncs[method] = fn
}

func (c *Conn) Run(ctx context.Context) error {
	for {
		select {
		case <-ctx.Done():
			return ctx.Err()

		default:
			// do nothing
		}

		raw, err := readMessage(c.reader)
		if err != nil {
			if err == io.EOF {
				return nil
			}

			return err
		}

		var env envelope
		if err := json.Unmarshal(raw, &env); err != nil {
			continue
		}

		go c.dispatch(ctx, env)
	}
}

func (c *Conn) dispatch(ctx context.Context, env envelope) {
	switch {
	case env.Method != "" && env.ID != nil:
		c.handlerMu.RLock()
		fn, ok := c.requestFuncs[env.Method]
		c.handlerMu.RUnlock()
		if !ok {
			c.writeResponse(&Response{
				JSONRPC: Version,
				ID:      env.ID,
				Error:   NewError(MethodNotFound, fmt.Sprintf("method not found: %s", env.Method)),
			})
			return
		}

		result, rpcErr := fn(ctx, c, env.Params)
		resp := &Response{JSONRPC: Version, ID: env.ID}
		if rpcErr != nil {
			resp.Error = rpcErr
		} else if result != nil {
			data, err := json.Marshal(result)
			if err != nil {
				resp.Error = NewError(InternalError, err.Error())
			} else {
				resp.Result = data
			}
		} else {
			resp.Result = json.RawMessage("null")
		}

		c.writeResponse(resp)

	case env.Method != "" && env.ID == nil:
		c.handlerMu.RLock()
		fn, ok := c.notifyFuncs[env.Method]
		c.handlerMu.RUnlock()
		if ok {
			fn(ctx, c, env.Params)
		}

	case env.Method == "" && env.ID != nil:
		key := env.ID.String()
		c.pendingMu.Lock()
		ch, ok := c.pending[key]
		if ok {
			delete(c.pending, key)
		}

		c.pendingMu.Unlock()
		if ok {
			ch <- &Response{JSONRPC: env.JSONRPC, ID: env.ID, Result: env.Result, Error: env.Error}
		}

	}
}

func (c *Conn) writeResponse(resp *Response) {
	data, err := json.Marshal(resp)
	if err != nil {
		return
	}

	c.writeMu.Lock()
	defer c.writeMu.Unlock()
	_ = writeMessage(c.writer, data)
}

func (c *Conn) Notify(method string, params any) error {
	p, err := json.Marshal(params)
	if err != nil {
		return err
	}

	req := &Request{JSONRPC: Version, Method: method, Params: p}
	data, err := json.Marshal(req)
	if err != nil {
		return err
	}

	c.writeMu.Lock()
	defer c.writeMu.Unlock()
	return writeMessage(c.writer, data)
}

func (c *Conn) Call(ctx context.Context, method string, params any, result any) error {
	c.pendingMu.Lock()
	c.nextID++
	id := NewIntID(c.nextID)
	ch := make(chan *Response, 1)
	c.pending[id.String()] = ch
	c.pendingMu.Unlock()

	p, err := json.Marshal(params)
	if err != nil {
		return err
	}

	req := &Request{JSONRPC: Version, ID: &id, Method: method, Params: p}
	data, err := json.Marshal(req)
	if err != nil {
		return err
	}

	c.writeMu.Lock()
	err = writeMessage(c.writer, data)
	c.writeMu.Unlock()
	if err != nil {
		return err
	}

	select {
	case resp := <-ch:
		if resp.Error != nil {
			return resp.Error
		}

		if result != nil && resp.Result != nil {
			return json.Unmarshal(resp.Result, result)
		}

		return nil
	case <-ctx.Done():
		return ctx.Err()
	}
}
