package cli

import "github.com/spf13/viper"

type CliFormat int

const (
	CliFormatPretty CliFormat = iota
	CliFormatJson
)

const DefaultCliFormat CliFormat = CliFormatPretty

func GetCliFormat() CliFormat {
	if enabled := viper.GetBool("json"); enabled {
		return CliFormatJson
	}

	format := viper.GetString("format")
	switch format {
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
	case CliFormatJson:
		return "json"

	case CliFormatPretty:
		return "pretty"

	default:
		return "unknown"
	}
}
