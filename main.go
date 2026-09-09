package main

import (
	"os"

	"taskfile-lsp/internal/cli"
)

func main() {
	if err := cli.RootCommand.Execute(); err != nil {
		os.Exit(1)
	}
}
