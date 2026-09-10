package cli

import (
	"fmt"
	"taskfile-lsp/internal/lsp"

	"github.com/spf13/cobra"
)

func HandleVersion(cmd *cobra.Command, args []string) error {
	fmt.Printf("%s\n", lsp.GetVersion())
	return nil
}

func init() {
	version := &cobra.Command{
		Use:   "version",
		Short: "Show version",
		RunE:  HandleVersion,
	}
	RootCommand.AddCommand(version)
}
