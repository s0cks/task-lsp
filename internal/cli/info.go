package cli

import (
	"encoding/json"
	"fmt"
	"taskfile-lsp/internal/lsp"

	"github.com/spf13/cobra"
)

func HandleInfo(cmd *cobra.Command, args []string) error {
	format := GetCliFormat()
	switch format {
	case CliFormatJson:
		info := map[string]any{}
		info["version"] = lsp.GetVersion()

		bytes, err := json.MarshalIndent(info, "", "  ")
		if err != nil {
			return fmt.Errorf("failed to marshal info json: %v", err)
		}

		fmt.Println(string(bytes))

	case CliFormatPretty:
		fmt.Printf("version: %s", lsp.GetVersion())

	default:
		return fmt.Errorf("invalid output format: %s", format.String())
	}

	return nil
}

func init() {
	info := &cobra.Command{
		Use:   "info",
		Short: "Show runtime info",
		RunE:  HandleInfo,
	}
	info.Flags().StringP("format", "f", "pretty", "The output format")
	info.Flags().BoolP("json", "", false, "Enable json output")
	info.MarkFlagsMutuallyExclusive("format", "json")

	RootCommand.AddCommand(info)
}
