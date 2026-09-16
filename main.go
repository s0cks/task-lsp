package main

import (
	"fmt"
	"os"

	"taskfile-lsp/internal/cli"
)

func main() {
	if err := cli.RootCommand.Execute(); err != nil {
		fmt.Println()
		fmt.Printf("error: %v", err)
		fmt.Println()
		os.Exit(1)
	}
}
