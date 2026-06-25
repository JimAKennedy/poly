# Poly Guide Site

Project-local skill for the Poly documentation site built with Astro + Starlight.

## When to use

Trigger on any work touching `site/` — content edits, styling, components, config, or deploy pipeline.

## Architecture

- **Framework**: Astro 5 + Starlight (docs theme)
- **Config**: `site/astro.config.mjs`
- **Base path**: `/poly` (GitHub Pages subdirectory)
- **Deploy**: GitHub Actions (`deploy-site.yml`) on push to `main` when `site/**` changes
- **Live URL**: https://jimakennedy.github.io/poly/

## File layout

```
site/
  astro.config.mjs          # Starlight integration config, sidebar, social
  src/
    components/              # Astro component overrides + custom components
      Banner.astro           # Global banner (Starlight component override)
      CodeSnippet.astro
      EuclideanDiagram.astro
      ListenFor.astro
      PolyPatch.astro
      PolyScreenshot.astro
    content/docs/            # MDX content pages (chapters, appendices)
    styles/custom.css        # All visual identity: palette, fonts, dark mode
    content.config.ts        # Content collection schema
  public/screenshots/        # Static assets
```

## Critical gotchas

### Starlight global config vs per-page frontmatter

Starlight has two configuration surfaces that are easy to confuse:

1. **Integration config** (`starlight({...})` in `astro.config.mjs`) — controls sidebar, title, social links, `customCss`, and `components` overrides. Does NOT support `banner`, `head`, or per-page options.

2. **Page frontmatter** (YAML in each `.mdx` file) — controls `title`, `description`, `banner`, `head`, `tableOfContents`, etc. These are per-page only.

**To add something globally**, use the `components` override pattern: create an Astro component and register it in `astro.config.mjs` under `components: { ComponentName: './src/components/Foo.astro' }`. The current `Banner.astro` uses this pattern.

**Never put `banner` in the integration config** — it will fail silently or break the build depending on the Starlight version.

### Dark mode variables

Starlight uses `[data-theme='dark']` on the root element. The CSS variable system has two layers:

1. **Starlight variables** (`--sl-color-*`) — must be overridden in `:root[data-theme='dark']` block. Key ones: `--sl-color-black` (page background), `--sl-color-white` (primary text), `--sl-color-gray-1` through `--sl-color-gray-6` (full gray scale), accents.

2. **Poly custom properties** (`--poly-*`) — must also have dark variants in the same block.

If you add a new custom property in `:root`, always add its dark variant in `:root[data-theme='dark']`.

### Component colors

Never hardcode hex colors in `.astro` components. Use CSS custom properties (`var(--poly-*)` or `var(--sl-color-*)`) so dark mode works automatically.

## Typography

- **Body/headings**: Source Serif 4 Variable (serif)
- **UI/tables/captions**: Inter Variable (sans)
- **Code**: JetBrains Mono Variable
- **Measure**: 65ch max-width on wide screens

## Deploy pipeline

The workflow (`deploy-site.yml`) runs on:
- Push to `main` when `site/**` or the workflow file changes
- Manual trigger via `workflow_dispatch`

Requires GitHub Pages source set to **GitHub Actions** (not "Deploy from a branch") in repo settings.

## Design system alignment

The `audio-meta` repo (`~/dev/audio-meta/design/`) has brand tokens, color/type/spacing CSS variables, and component reference implementations for the jk.digital portfolio. When aligning Poly's site with the portfolio design system, pull tokens from there rather than inventing new ones.

## Content structure

- 15 chapters covering polymetric drumming traditions and techniques
- 3 appendices: preset reference, Euclidean reference, bibliography
- References page has ~45 numbered citations linked from inline superscripts in chapters

## Local dev

```bash
cd site && npm run dev    # starts on localhost:4321
cd site && npm run build  # production build to site/dist/
```
