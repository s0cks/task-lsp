package cli

import (
	"strings"

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
}

func init() {
	RootCommand.AddGroup(&cobra.Group{
		ID:    "misc",
		Title: "Misc",
	})
}
