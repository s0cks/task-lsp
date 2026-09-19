package cli

import (
	"os"

	"charm.land/log/v2"
	"github.com/spf13/viper"
)

func getLogPath() string {
	path := viper.GetString("log-file")
	if path == "" {
		path = os.ExpandEnv("$HOME/.cache/taskfile-lsp.log")
	}

	return path
}

func createLogger() func() {
	logFile, err := os.OpenFile(getLogPath(), os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0o0666)
	if err != nil {
		panic(err)
	}

	log.SetOutput(logFile)
	log.SetFormatter(log.JSONFormatter)
	return func() {
		_ = logFile.Close()
	}
}
