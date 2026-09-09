package cli

import (
	"fmt"

	"github.com/spf13/cobra"
)

func HandleVersion(cmd *cobra.Command, args []string) error {
	fmt.Printf("%s\n", "0.0.0")
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
