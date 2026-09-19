// Renders one Mermaid source (read from stdin) through the site's own rehype
// pipeline and writes the resulting HTML to stdout. Used by
// mermaid-render.test.mjs to render in a fresh process, so "two builds" means
// two processes rather than two calls.
import { readFileSync } from 'node:fs';
import { unified } from 'unified';
import rehypeParse from 'rehype-parse';
import rehypeStringify from 'rehype-stringify';
import rehypeMermaid from 'rehype-mermaid';
import { mermaidRehypeOptions } from '../../src/lib/mermaid-config.mjs';

const source = readFileSync(0, 'utf8');
const html = `<pre><code class="language-mermaid">${source}</code></pre>`;

const file = await unified()
  .use(rehypeParse, { fragment: true })
  .use(rehypeMermaid, mermaidRehypeOptions)
  .use(rehypeStringify)
  .process(html);

process.stdout.write(String(file));
