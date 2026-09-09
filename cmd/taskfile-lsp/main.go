package main

import (
	"context"
	"fmt"
	"log"
	"os"

	"taskfile-lsp/internal/cli"
	"taskfile-lsp/internal/lsp"
	"taskfile-lsp/internal/rpc"
)

func main() {
	if len(os.Args) > 1 {
		if err := cli.RootCommand.Execute(); err != nil {
			fmt.Printf("failed to execute: %v", err)
			os.Exit(1)
		}

		os.Exit(0)
	}

	logger := log.New(os.Stderr, "taskfile-lsp ", log.LstdFlags)
	conn := rpc.NewConn(os.Stdin, os.Stdout)
	server := lsp.NewServer(logger)
	server.Register(conn)
	if err := conn.Run(context.Background()); err != nil {
		logger.Fatalf("connection closed: %v", err)
	}
}
