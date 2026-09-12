// vite.config.js
import { defineConfig } from 'vite';
import { svelte } from '@sveltejs/vite-plugin-svelte';
import { vitePreprocess } from '@sveltejs/vite-plugin-svelte';
import { mdsvex, escapeSvelte } from 'mdsvex';
import { createHighlighter } from 'shiki';

const highlighter = await createHighlighter({
  themes: ['github-dark'],
  langs: ['svelte', 'lua', 'bash', 'json', 'yaml', 'go']
});

export default defineConfig({
  plugins: [
    svelte({
      extensions: ['.svelte', '.svx', '.md'],
      preprocess: [
        mdsvex({
          extensions: ['.svx', '.md'],
          smartypants: false,
          highlight: {
            highlighter: async (code, lang) => {
              const html = highlighter.codeToHtml(code, {
                lang: lang || 'plaintext',
                theme: 'github-dark',
              });
              return escapeSvelte(html);
            }
          }
        }),
        vitePreprocess()
      ]
    })
  ],
  base: '/task-lsp/',
});
