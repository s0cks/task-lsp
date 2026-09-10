# taskfile-lsp

A [language-server-protocol](https://microsoft.github.io/language-server-protocol/) (LSP) implementation for [Taskfiles](https://taskfile.dev/).

![Example Document Symbols](./assets/lsp-document-symbols.gif)

## Features

Working features:

- Completion --- Complete task names, definitions and fields.
- Diagnostics --- Detect invalid task references.
- Document symbols --- Browse tasks and other declarations from your editor's symbol picker.
- Hover documentation --- Show task descriptions when hovering over task references.

Planned Features:

- Rename
- Find references
- Workspace symbols
- Formatting
- Schema validation
- Task dependency analysis

## Installation

Check out the [install guide](https://github.com/s0cks/task-lsp/wiki/InstallGuide) in the wiki for how to install.

## Editor setup

Check out the [editor setup guide](https://github.com/s0cks/task-lsp/wiki/EditorSetup) in the wiki for how to configure
for your editor.

> `task-lsp` should be able to work for any editor that supports the [language-server-protocol](https://microsoft.github.io/language-server-protocol/) (LSP).

## Credits

- The [task](https://taskfile.dev) team :heart_hands:

## LICENSE

See [LICENSE](/LICENSE)
