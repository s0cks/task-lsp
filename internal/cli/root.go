package cli

import (
	"context"
	"os"
	"strings"
	"taskfile-lsp/internal/lsp"
	"taskfile-lsp/internal/rpc"

	"charm.land/log/v2"
	"github.com/spf13/cobra"
	"github.com/spf13/viper"
)

func initConfig(cmd *cobra.Command) error {
	viper.SetEnvPrefix("TASKFILE_LSP")
	viper.SetEnvKeyReplacer(strings.NewReplacer("-", "_", ".", "_"))
	viper.AutomaticEnv()
	return viper.BindPFlags(cmd.Flags())
}

var RootCommand = &cobra.Command{
	Use:   "taskfile-lsp",
	Short: "A LSP for Task",
	PersistentPreRunE: func(cmd *cobra.Command, args []string) error {
		return initConfig(cmd)
	},
	RunE: func(cmd *cobra.Command, args []string) error {
		closeLogger := createLogger()
		defer closeLogger()

		log.Info("starting taskfile-lsp....")
		conn := rpc.NewConn(os.Stdin, os.Stdout)
		server := lsp.NewServer()
		server.Register(conn)
		if err := conn.Run(context.Background()); err != nil {
			log.Fatalf("connection closed: %v", err)
		}

		return nil
	},
}

func init() {
	RootCommand.PersistentFlags().StringP("log-file", "", "", "Set the log file")
}
