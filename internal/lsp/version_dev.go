//go:build dev

package lsp

import (
	"fmt"
)

func GetVersion() string {
	if Version[0] >= '0' && Version[0] <= '9' {
		return fmt.Sprintf("v%s-dev", Version)
	}

	return Version + "-dev"
}
