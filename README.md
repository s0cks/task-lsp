# taskfile-lsp

> A [language server protocol](https://microsoft.github.io/language-server-protocol/) (LSP) for [Task](https://taskfile.dev/) Taskfiles.

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

### Binary

You can download the latest from the [Releases](https://github.com/s0cks/task-lsp/releases) page or by using one of the following tools:

Release binaries follow the format:

`taskfile-lsp-<os>-<arch>`

For example:

|   OS    | Arch  | Name                           |
| :-----: | :---: | :----------------------------- |
|  Linux  | amd64 | taskfile-lsp-linux-amd64       |
|  Linux  | arm64 | taskfile-lsp-linux-arm64       |
|  MacOS  | amd64 | taskfile-lsp-macos-amd64       |
|  MacOS  | arm64 | taskfile-lsp-macos-arm64       |
| Windows | amd64 | taskfile-lsp-windows-amd64.exe |

### Linux and macOS

Download using curl:

```sh
curl \
  -L https://github.com/s0cks/task-lsp/releases/latest/download/taskfile-lsp-linux-amd64 \
  -o taskfile-lsp
```

Or using wget:

```sh
wget https://github.com/s0cks/task-lsp/releases/latest/download/taskfile-lsp-linux-amd64 \
  -O taskfile-lsp
```

Or using httpie:

```sh
https --download https://github.com/s0cks/task-lsp/releases/latest/download/taskfile-lsp-linux-amd64
```

Then, mark the binary as executable:

```sh
chmod +x taskfile-lsp
```

Then place it in your `$PATH`:

```sh
mv taskfile-lsp ~/.local/bin
```

For system-wide installs:

```sh
mv taskfile-lsp /usr/local/bin
```

### From source

Clone the repository:

```sh
git clone https://github.com/s0cks/task-lsp
cd task-lsp/
```

Build the executable:

```sh
go build -ldflags="-s -w" \
  -o taskfile-lsp         \
  cmd/taskfile-lsp/main.go
```

Finally, install it:

```sh
mv taskfile-lsp ~/.local/bin
```

> Make sure `~/.local/bin` is in your `$PATH`

## Editor setup

### Neovim

#### Using nvim-lspconfig

```lua
vim.lsp.config('taskfile', {
  cmd = { 'taskfile-lsp' },
  filetypes = { 'yaml.taskfile', 'taskfile' },
  root_markers = { 'Taskfile.yaml', 'Taskfile.yml' },
})
vim.lsp.enable('taskfile')
```

## Credits

- The [task](https://taskfile.dev) team :heart_hands:

## LICENSE

See [LICENSE](/LICENSE)
