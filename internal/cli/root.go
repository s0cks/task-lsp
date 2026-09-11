package cli

import (
	"context"
	"fmt"
	"log"
	"os"
	"strings"
	"taskfile-lsp/internal/lsp"
	"taskfile-lsp/internal/rpc"

	"github.com/spf13/cobra"
	"github.com/spf13/viper"
)

func initConfig(cmd *cobra.Command) error {
	viper.SetEnvPrefix("TASKFILE_LSP")
	viper.SetEnvKeyReplacer(strings.NewReplacer("-", "_", ".", "_"))
	viper.AutomaticEnv()
	return viper.BindPFlags(cmd.Flags())
}

var logFile *os.File
var logger *log.Logger

var RootCommand = &cobra.Command{
	Use:   "taskfile-lsp",
	Short: "A LSP for Task",
	PersistentPreRunE: func(cmd *cobra.Command, args []string) error {
		return initConfig(cmd)
	},
	PreRunE: func(cmd *cobra.Command, args []string) error {
		logfilePath := os.ExpandEnv("$HOME/.cache/taskfile-lsp.log")
		logFile, err := os.OpenFile(logfilePath, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0666)
		if err != nil {
			return fmt.Errorf("failed to open log file: %v", err)
		}

		logger = log.New(logFile, "", log.LstdFlags|log.Lshortfile)
		return nil
	},
	RunE: func(cmd *cobra.Command, args []string) error {
		logger.Println("This log message is saved directly to app.log")

		conn := rpc.NewConn(os.Stdin, os.Stdout)
		server := lsp.NewServer(logger)
		server.Register(conn)
		if err := conn.Run(context.Background()); err != nil {
			logger.Fatalf("connection closed: %v", err)
		}

		return nil
	},
	PostRunE: func(cmd *cobra.Command, args []string) error {
		if logFile != nil {
			if err := logFile.Close(); err != nil {
				return fmt.Errorf("failed to close log file: %v", err)
			}
		}

		return nil
	},
}

func init() {
	RootCommand.AddGroup(&cobra.Group{
		ID:    "misc",
		Title: "Misc",
	})
	RootCommand.PersistentFlags().StringP("log-file", "", "", "Set the log file")
}
