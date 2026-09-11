Output ./assets/diagnostic-invalid-task-ref.gif
Source ./assets/_neovim_taskfile.tape

Type ":Telescope lsp_document_symbols" Sleep 500ms Enter
Sleep 250ms
Type "clean" Sleep 500ms Enter
Sleep 250ms
Down 2

Type "o- task: invalid-task-ref" Sleep 500ms Escape
Sleep 1s

Source ./assets/_close_neovim.tape
