package main

import (
	"fmt"
	"os"

	"taskfile-lsp/internal/cli"
)

func main() {
	if err := cli.RootCommand.Execute(); err != nil {
		fmt.Printf("failed to execute: %v", err)
		os.Exit(1)
	}
}
