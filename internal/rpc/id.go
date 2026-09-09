package rpc

import (
	"encoding/json"
	"fmt"
	"strconv"
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
