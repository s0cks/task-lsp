package cli

import (
	"fmt"
	"os"
	"taskfile-lsp/internal/document"

	"github.com/spf13/cobra"
	"github.com/spf13/viper"
)

func handleTestParser(cmd *cobra.Command, args []string) error {
	source := viper.GetString("source")
	f, err := os.ReadFile(source)
	if err != nil {
		return err
	}

	var doc *document.Document
	fmt.Println("Tasks:")
	doc, err = document.ParseDocumentString(string(f))
	if err != nil {
		return err
	}

	doc.VisitTasks(func(idx uint64, task document.Task) bool {
		var hasTrailingComment = "N"
		if task.HasTrailingComment() {
			hasTrailingComment = "Y"
		}

		fmt.Printf("- %s - %s\n", task.Name(), hasTrailingComment)

		if task.GetNumberOfComments() > 0 {
			for i := range task.GetNumberOfComments() {
				c := task.GetCommentAt(i)
				fmt.Printf("  - %s\n", c.Value())
			}
		}

		if task.HasTrailingComment() {
			c := task.GetTrailingComment()
			fmt.Printf("  - %s\n", c.Value())
		}

		return true
	})

	return nil
}

func init() {
	command := &cobra.Command{
		Use:  "test-parser",
		RunE: handleTestParser,
	}
	command.Flags().String("source", "", "The source to test")
	_ = command.MarkFlagRequired("source")
	RootCommand.AddCommand(command)
}
