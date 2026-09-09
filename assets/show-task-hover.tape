Output ./assets/show-task-hover.gif
Source ./assets/_neovim_taskfile.tape

Type ":Telescope lsp_document_symbols" Sleep 500ms Enter
Sleep 250ms
Type "vhs" Sleep 500ms Enter
Sleep 250ms
Type "K"
Sleep 1s

Type ":Telescope lsp_document_symbols" Sleep 500ms Enter
Sleep 250ms
Type "build-release" Sleep 500ms Enter
Sleep 250ms
Down 4
Type "fm"
Sleep 500ms
Type "K"
Sleep 1s

Source ./assets/_close_neovim.tape
