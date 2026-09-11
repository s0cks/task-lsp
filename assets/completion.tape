Output ./assets/completion.gif
Source ./assets/_neovim_taskfile.tape

Type ":Telescope lsp_document_symbols" Sleep 500ms Enter
Sleep 250ms
Type "clean" Sleep 500ms Enter
Sleep 250ms
Down 2

Type "o- task: g" Sleep 1s
Down 4
Sleep 500ms
Ctrl+Enter

Source ./assets/_close_neovim.tape
