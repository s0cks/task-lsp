import * as path from 'path';
import { ExtensionContext, workspace } from 'vscode';
import { LanguageClient, LanguageClientOptions, ServerOptions } from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: ExtensionContext) {
    const serverBinary = context.asAbsolutePath(
        path.join('bin', 'taskfile-lsp')
    );

    const serverOptions: ServerOptions = {
        run: { command: serverBinary, args: [] },
        debug: { command: serverBinary, args: [] }
    };

    const clientOptions: LanguageClientOptions = {
       documentSelector: [
            { scheme: 'file', language: 'yaml', pattern: '**/Taskfile.yaml' },
            { scheme: 'file', language: 'yaml', pattern: '**/Taskfile.yml' },
            { scheme: 'file', language: 'yaml', pattern: '**/taskfile.yaml' },
            { scheme: 'file', language: 'yaml', pattern: '**/taskfile.yml' }
        ],
        synchronize: {
            // Notify your Go LSP when changes are committed to a Taskfile
            fileEvents: workspace.createFileSystemWatcher('**/[Tt]askfile.y*ml')
        }
    };

    client = new LanguageClient(
        'taskfile-lsp',
        'A taskfile language-server (LSP) in Go',
        serverOptions,
        clientOptions
    );

    client.start();
}

export function deactivate(): Thenable<void> | undefined {
    return client ? client.stop() : undefined;
}
