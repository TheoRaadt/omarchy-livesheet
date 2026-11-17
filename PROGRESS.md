# Progress

Tracks what's done, per feature, against `README.md`'s spec. Everything
below was verified by building (`bin/build`), running the backend tests
(`bin/test`), and driving the actual compiled app with real keystrokes
(not just reading the source).

## Done

- **Headings** — `#`/`##`/`### ` at the start of a line convert it to a
  Title (large, bold, centered) / Paragraph (bold) / Subparagraph (bold
  italic). Verified live: typing `# ` then text renders a large centered
  heading.
- **Live math fields** — default line type; numbers, letters, operators
  render through MathQuill as you type.
- **Variable assignment (`:` → `:=`)** — `a:3` renders as `a := 3` and
  the value is available to later lines. Verified live.
- **Fractions (`/`)** — opens a two-placeholder fraction, Tab/Shift+Tab
  moves between numerator and denominator.
- **Square root (`\`)** — inserts a root with an empty radicand.
- **Nth root (`Shift+\` / `|`)** — inserts an index + radicand template
  linked by Tab/Shift+Tab. Verified live.
- **Evaluation (`=`)** — evaluates the line, resolving variables defined
  on earlier lines. Verified live: `a:3`, `b:27`, then the *a*-th root of
  *b* followed by `=` correctly gave `3`.
- **Line navigation** — Enter starts a new line; Backspace at the start
  of an empty line removes it and returns to the end of the previous one.
- **Matrices (`[`)** — grid of MathQuill cells with rendered brackets.
  - `,` adds a column, `;` finishes a row and starts a new one.
  - Tab/Shift+Tab and arrow keys move between cells, crossing into the
    adjacent cell at content edges (Left/Right) or jumping a row at the
    same column (Up/Down).
  - Stepping out at the matrix's outer edge hands off to the
    notebook line before/after it — forward always lands somewhere
    (creates a line if needed, like Enter), backward only moves to an
    existing line.
  - Backspace on an empty trailing cell removes it; backspacing the last
    cell of a 1×1 matrix reverts the line to plain/pending.
  - Enter finishes the matrix and starts a new line below it.
  - Exports as `\begin{bmatrix}...\end{bmatrix}`.
  - Verified live: `[1,2;3,4]` rendered as a correct bracketed 2×2 grid.
- **Export (`Ctrl+S`)** — opens a dialog (`src/Main.qml`) to export the
  notebook to `.tex` (headings → `\paragraph`/`\subparagraph`/centered
  bold line, math → centered `\[ \]` display equations, matrices →
  `bmatrix`, empty non-math lines dropped) or straight to `.pdf` via
  `pdflatex` (`src/backend.cpp`). Covered by 4 backend unit tests, all
  passing.
- **Quit (`Ctrl+Q`)**.
- **Theme following** — `Theme` (`src/theme.cpp`) reads the live accent
  color, with a documented fallback (`#FFD60A` / black foreground) when
  no `colors.toml` is present. Covered by a passing unit test.
- **Build/test/install scripts** — `bin/build` (qmake6 + make),
  `bin/test` (offscreen QtTest run), `bin/install` (`makepkg -fsi`).

## Not done / out of scope (per README's own "Notes" section)

- No i18n (`qsTr`/translation catalogs) — explicitly skipped.
- No quit-confirmation-on-unsaved-work — explicitly skipped.

## Verification log

- 2026-11-17: `bin/build` and `bin/test` both clean (7/7 tests pass).
  Ran the compiled binary under Wayland/Hyprland, drove it with `wtype`
  for headings, variables, nth-root evaluation, and a 2×2 matrix;
  screenshotted each with `grim` and confirmed correct rendering.
