Output ./assets/goto-task-definition.gif
Source ./assets/_neovim_taskfile.tape

Type ":Telescope lsp_document_symbols" Sleep 500ms Enter
Sleep 250ms
Type "clean" Sleep 500ms Enter
Sleep 250ms
Down 3

Type "fg" Sleep 500ms Escape
Sleep 1s

Type "gd" Sleep 500ms Escape
Sleep 1s

Source ./assets/_close_neovim.tape
