package lsp

import (
	"fmt"
)

var Version = "development"

func GetVersion() string {
	if Version == "development" {
		return "development"
	}

	return fmt.Sprintf("v%s", Version)
}
