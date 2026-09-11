package cli

import (
	"encoding/json"
	"fmt"
	"taskfile-lsp/internal/lsp"

	"github.com/spf13/cobra"
)

type CliVersionHandler func() error

func handleInfoJson() error {
	info := map[string]any{}
	info["version"] = lsp.GetVersion()

	bytes, err := json.MarshalIndent(info, "", "  ")
	if err != nil {
		return fmt.Errorf("failed to marshal info json: %v", err)
	}

	fmt.Println(string(bytes))
	return nil
}

func handleInfoPretty() error {
	fmt.Printf("version: %s\n", lsp.GetVersion())
	return nil
}

func getInfoHandler() (CliVersionHandler, error) {
	format := GetCliFormat()
	switch format {
	case CliFormatJson:
		return handleInfoJson, nil

	case CliFormatPretty:
		return handleInfoPretty, nil

	default:
		return nil, fmt.Errorf("invalid output format: %s", format.String())
	}
}

func HandleInfo(cmd *cobra.Command, args []string) error {
	handler, err := getInfoHandler()
	if err != nil {
		return err
	}

	if err := handler(); err != nil {
		return err
	}

	fmt.Println()
	return nil
}

func init() {
	info := &cobra.Command{
		Use:   "info",
		Short: "Show runtime info",
		RunE:  HandleInfo,
	}
	AddFormatFlags(info)

	RootCommand.AddCommand(info)
}
