# livesheet

A one-window, keyboard-only math notebook for Omarchy. There's no raw
markup to see — you type directly into a live, textbook-rendered canvas
(powered by MathQuill).

## How it works

Each line starts out undecided and becomes one of these the moment you
type into it:

- `# ` at the start of a line → a document **Title** (large, centered)
- `## ` → a **Paragraph** heading (bold)
- `### ` → a **Subparagraph** heading (bold italic)
- Anything else → a **live math field**

Inside a math field:

- Numbers, letters, and operators render as proper math as you type
  (MathQuill)
- `:` becomes `:=` — for defining a variable, e.g. `a:2` renders as `a := 2`
- `/` opens a fraction: two empty, stacked placeholders (numerator and
  denominator) that you fill in with numbers or variables, moving between
  them with **Tab** / **Shift+Tab**
- `\` inserts a square root with an empty radicand
- `Shift+\` (`|`) inserts an nth root: an index placeholder and a radicand
  placeholder, also connected by **Tab** / **Shift+Tab**
- `=` evaluates everything typed so far on the line into a number, e.g.
  the cube-root template filled with `3` and `27` followed by `=` shows
  `= 3`. It also resolves variables defined on earlier lines — `a:3` then
  `b:27` then the *a*-th root of `b` followed by `=` also gives `3`
- **Enter** starts a new line; **Backspace** at the start of an empty line
  removes it and returns you to the end of the previous one

`[` at the start of a line starts a **matrix** instead of a math field:

- `,` adds another column (cell) to the current row
- `;` finishes the row and starts a new one below it
- **Tab** / **Shift+Tab**, or the **arrow keys**, move between cells —
  Left/Right cross into the adjacent cell once you reach the edge of the
  current one's content, Up/Down jump a row at the same column
- At the matrix's outer edge, the same keys step out to the notebook line
  before/after it. Stepping out forward (Tab, ArrowDown, Right) always
  lands somewhere — it creates a new line if none exists yet, the same as
  Enter. Stepping out backward (Shift+Tab, ArrowUp, Left) only moves to a
  line that's already there
- **Backspace** on an empty trailing cell removes it; backspacing the very
  last cell of a 1×1 matrix turns the line back into a plain empty line,
  ready to become whatever you type next (a heading, a math line, or
  another matrix)
- **Enter** finishes the matrix and starts a new notebook line below it

Matrix cells hold plain numbers, variables, or expressions — they don't
support the `:`, `\`, `|`, or `=` shortcuts the way a regular math line
does. Exported as a standard `\begin{bmatrix}...\end{bmatrix}`.

## Shortcuts

| Key      | Action   |
|----------|----------|
| `Ctrl+S` | Export   |
| `Ctrl+Q` | Quit     |

## Export

`Ctrl+S` opens a small dialog to export the whole notebook — headings as
LaTeX sectioning commands, math lines as centered display equations — to a
`.tex` file, or straight to a compiled `.pdf` via `pdflatex`.

## Build

```sh
bin/build   # qmake6 + make -> build/livesheet
bin/test    # backend unit tests (JSON parsing, LaTeX export), offscreen
bin/install # makepkg -fsi, adds it to the app launcher
```

Requires `qt6-base`, `qt6-declarative`, `qt6-webengine`, `qt6-wayland`,
`xdg-desktop-portal`, and `texlive` (for PDF export).

## Notes

This app skips Omarchy's usual translation layer (`qsTr`/i18n catalogs) and
the quit-confirmation-on-unsaved-work convention for now — it's a
single-user tool, not a published app, so keeping scope down took priority.
The live theme accent (`Theme`) is still followed. The whole notebook lives
in the WebEngineView (`src/notebook.html`); Qt/QML only hosts the window
and handles file export.
