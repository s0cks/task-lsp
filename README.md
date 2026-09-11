# taskfile-lsp

A [language-server-protocol](https://microsoft.github.io/language-server-protocol/) (LSP) implementation for [Taskfiles](https://taskfile.dev/).

## Features

|       Feature       |                             Description                             |                             Preview                              |
| :-----------------: | :-----------------------------------------------------------------: | :--------------------------------------------------------------: |
|     Completion      |              Complete task and var names, fields, etc               |          ![Completion Preview](./assets/completion.gif)          |
|     Diagnostics     |                   Detect invalid task references                    | ![Diagnostics Preview](./assets/diagnostic-invalid-task-ref.gif) |
|  Document Symbols   | Browse tasks and other declarations from your editors symbol picker |      ![Symbols Preview](./assets/lsp-document-symbols.gif)       |
| Hover Documentation |      Show task descriptions when hovering over task references      |          ![Hover Preview](./assets/show-task-hover.gif)          |
|     References      |                                 TBD                                 |                               TBD                                |

Planned Features:

- Rename
- Workspace symbols
- Task dependency analysis
- Formatting
- Schema validation

## Installation

Check out the [install guide](https://github.com/s0cks/task-lsp/wiki/InstallGuide) in the wiki for how to install.

## Editor setup

Check out the [editor setup guide](https://github.com/s0cks/task-lsp/wiki/EditorSetup) in the wiki for how to configure
for your editor.

> `task-lsp` should be able to work for any editor that supports the [language-server-protocol](https://microsoft.github.io/language-server-protocol/) (LSP).

## Credits

- The [task](https://taskfile.dev) team :heart_hands:

## License

See [LICENSE](/LICENSE)
