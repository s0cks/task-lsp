# taskfile-lsp

A [language-server-protocol](https://microsoft.github.io/language-server-protocol/) (LSP) implementation for [Taskfiles](https://taskfile.dev/).

Get IDE features like completion, diagnostics, code actions, symbols, hover documentation & more while working with
your Taskfiles

![Demo](./assets/goto-task-definition.gif)

## Features

|       Feature       |                             Description                             |
| :-----------------: | :-----------------------------------------------------------------: |
|     Completion      |              Complete task and var names, fields, etc               |
|     Diagnostics     |                   Detect invalid task references                    |
|  Document Symbols   | Browse tasks and other declarations from your editors symbol picker |
| Hover Documentation |      Show task descriptions when hovering over task references      |
|     References      |                                 TBD                                 |

## Installation

Check out the [install guide](https://github.com/s0cks/task-lsp/wiki/InstallGuide) in the wiki for how to install.

## Editor setup

Check out the [editor setup guide](https://github.com/s0cks/task-lsp/wiki/EditorSetup) in the wiki for how to configure
for your editor.

> `task-lsp` should be able to work for any editor that supports the [language-server-protocol](https://microsoft.github.io/language-server-protocol/) (LSP).

| Editor |   Status    |
| :----: | :---------: |
| Neovim |    Good     |
| VSCode | Coming Soon |
|  Zed   |     ---     |

## Roadmap

Planned Features:

- Rename
- Workspace symbols
- Task dependency analysis
- More code actions
- Formatting
- Schema validation

## Contributing

Contributions are welcome.

Please refer to the [contribution guide](https://github.com/s0cks/task-lsp/wiki/ContributingGuide) in the wiki for how
to get started.

## Credits

- The [task](https://taskfile.dev) team :heart_hands:
- [@paulvarache](https://github.com/paulvarache) for [the original](https://github.com/paulvarache/taskfile-language-server) implementation :heart_hands:

## License

See [LICENSE](/LICENSE)
