package cli

import (
	"fmt"
	"strings"

	"github.com/spf13/cobra"
	"github.com/spf13/viper"
)

type CliFormat int

const (
	CliFormatPlain CliFormat = iota
	CliFormatPretty
	CliFormatJson
)

func GetAllCliFormats() []string {
	return []string{
		"plain",
		"pretty",
		"json",
	}
}

const DefaultCliFormat CliFormat = CliFormatPretty

func GetCliFormat() CliFormat {
	if enabled := viper.GetBool("json"); enabled {
		return CliFormatJson
	}

	format := viper.GetString("format")
	switch format {
	case "plain":
		return CliFormatPlain
	case "json":
		return CliFormatJson
	case "pretty":
		return CliFormatPretty
	default:
		return DefaultCliFormat
	}
}

func (format CliFormat) String() string {
	switch format {
	case CliFormatPlain:
		return "plain"

	case CliFormatJson:
		return "json"

	case CliFormatPretty:
		return "pretty"

	default:
		return "unknown"
	}
}

func AddFormatFlags(cmd *cobra.Command) {
	all := GetAllCliFormats()
	cmd.Flags().StringP("format", "f", "pretty", fmt.Sprintf("The output format. Valid values are: %s",
		strings.Join(all, ", "),
	))
	for _, format := range all {
		cmd.Flags().BoolP(format, "", false, fmt.Sprintf("Enable %s output", format))
	}
	cmd.MarkFlagsMutuallyExclusive(append(all, "format")...)
}
