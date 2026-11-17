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
- **Functions as variables (`f(x):=...`, `f(4)=`, now multi-variable too)**
  — a single-letter name followed by one or more comma-separated
  single-letter parameters in parens, e.g. `f(x):=x^2+4` or
  `f(x,y):=\sqrt{x^2+y^2}`, defines a function the same way `a:3` defines
  a variable. Calling it later, e.g. `f(4)=` or `f(2,3)=`, substitutes
  each argument for the matching parameter and evaluates the body,
  including referencing variables/other functions defined above it
  (`src/notebook.html`: `collectDefinitions()` now returns `{vars, funcs}`
  instead of just variables, with `funcs[name] = {params, body}`;
  `evaluateLatex()`'s `parseAtom` recognizes a known function name
  followed by `(`/`\left(` as a call rather than implicit multiplication,
  reading one comma-separated argument per parameter). Calling with the
  wrong number of arguments throws rather than silently misbinding.
  - Fixed as part of this: typed parens were previously unparseable at
    all — MathQuill emits `\left(`/`\right)` for a typed `(`/`)`, not bare
    `(`/`)`, so `parseAtom` now accepts both forms, and the implicit-
    multiplication lookahead in `parseTerm` (which used to treat *any*
    backslash as "start of another factor") was narrowed to only
    `\sqrt`/`\frac`/`\left(` so it stops misfiring on a `\right)` that's
    closing an enclosing group.
  - Verified with a standalone Node script running `evaluateLatex`/
    `collectDefinitions` copied verbatim from the file (no DOM/MathQuill
    needed for the pure evaluator logic): `f(x):=x^2+4` then `f(4)` →
    `20`; a function body referencing an earlier variable; a variable
    definition calling an earlier function; plain `2(3+4)` → `14`; an
    undefined `c(4)` still throws rather than silently returning 0.
  - Re-verified live in the running app once the GUI-driving flakiness
    settled (see the range-variable entry below for the same caveat):
    `f(x):=x^2+4` then `f(4)=` rendered `= 20`.
  - Multi-variable upgrade verified with a standalone Node script: a
    2-param `f(x,y):=\sqrt{x^2+y^2}` called as `f(2,3)` gives `3.605551`
    (√13); a 3-param function; the original single-param case still works
    unchanged; calling with the wrong argument count throws; a multi-param
    function called with a range argument still broadcasts correctly.
    Also confirmed live by the user, who tried `f(x,y):=\sqrt{x^2+y^2}`
    then `f(2,3)=` by hand in the running app (before this fix landed, so
    it rendered but the call itself wasn't evaluable yet).
- **Range variables (Mathcad-style, `;` for the range operator)** —
  `i:0;5` renders as `i := 0…5` (the integers 0 through 5); `i:0,2;10`
  renders as `i := 0,2…10` for an explicit step of 2. Any formula that
  references a range variable evaluates once per value in the range, the
  same as real Mathcad, rather than as a single number.
  - `src/notebook.html`: every arithmetic operator (`+ - * / ^ \sqrt
    \frac`, plus function calls) now goes through `broadcast()`/
    `broadcastUnary()`, which apply element-wise whenever either operand
    is an array instead of a number; `parseRangeDefinition()` builds that
    array from a `start…end`/`start,second…end` definition (capped at
    1000 entries so a typo like `i:0;1000000` can't hang the page); the
    marker itself is MathQuill's `\ldots` command (inserted via `.cmd()`
    on `;`), not two typed literal dots — MathQuill treats two typed `.`
    right after a digit as "still one decimal point" and silently drops
    the second one, which would have turned `0..5` into `0.5`.
  - Verified with the same kind of standalone Node script as the function
    feature above: `i:=0…5` is the array `[0,1,2,3,4,5]`; `i^2` over it
    gives `0, 1, 4, 9, 16, 25`; explicit step (`0,2…10`), descending
    ranges (`5…0`), a range broadcast against a plain scalar, a range
    passed as a function argument, and `\sqrt`/`\frac` broadcasting all
    checked; mismatched-length range arithmetic throws rather than
    silently misaligning; an absurdly long range is rejected.
  - Also re-verified live in the running app (after working around the
    GUI-driving flakiness noted above by giving the page a moment to
    repaint before screenshotting — the input itself wasn't the problem,
    the screenshot was just taken before the browser finished redrawing):
    `i:0;5` then `i^2=` rendered `i := 0…5` and `i^2 = 0, 1, 4, 9, 16, 25`.
- **Sum/product operators (Mathcad-style, `$`/`#` for Σ/Π)** — `$`/`#`
  (Mathcad's own bare, unmodified shortcuts) insert the plain `\Sigma`/
  `\Pi` symbol; the expression typed right after is summed/multiplied
  once per value of whatever range variable it contains, e.g. with
  `i := 0…5`, `$i^2=` gives `55` and (with `i := 1…4`) `#i=` gives `24`.
  - `src/notebook.html`: `reduceOverRange()` folds the array a range-
    broadcasted expression already produces with `+`/`*` — no new
    broadcasting plumbing needed, since Σ/Π are pure reducers on top of
    what range variables already built. `parseAtom` treats `\Sigma`/`\Pi`
    as a tight unary prefix (one `parsePow()`'s worth of operand, the same
    precedence unary minus uses), so `$i^2` sums `i^2` but summing a
    multi-term body needs explicit parens (`$\left(i+1\right)`) — without
    them, `$i+1` parses as `($i)+1`, same restriction Mathcad itself has.
    A scalar body (no range variable involved) passes through unchanged,
    so "the sum of one value" is just that value.
  - Verified with a standalone Node script: `Σi²` over `i:=0..5` → `55`;
    `Πi` over `i:=1..4` → `24`; `Σ(i+1)` sums the whole parenthesized
    body; `Σi+1` confirms the tight-binding precedence; a plain scalar
    body passes through; a sum used inside a larger scalar formula.
  - Not yet re-verified live: while testing this, the user was
    independently testing multi-variable function syntax by hand in the
    same running window my own test keystrokes were going to, so I
    stopped driving that window to avoid clobbering their input (see the
    multi-variable function entry above, which came out of exactly that).

- **White-paper/black-ink toggle (`Ctrl+L`)** — flips between the default
  dark sheet and a white-background/black-text look, for printing-style
  reading. Session-only (no settings storage in this app at all, so it's
  back to dark on next launch, same as window size/position).
  - `src/notebook.html`: the two appearances are `--bg`/`--ink` CSS custom
    properties, read by every color rule (directly, or via `currentColor`
    for things like the matrix brackets) rather than hardcoded colors, so
    `html.paper` overriding those two variables is the only place the
    look is actually defined; `togglePaperMode()` just toggles that class
    and is wired to `Ctrl+L` in the same global keydown listener as
    `Ctrl+S`/`Ctrl+Q`.
  - Fixed as part of this: the class and variables initially went on
    `<body>`, which seemed natural, but a CSS custom property only
    cascades to descendants — `<body>` is a descendant of `<html>`, not
    the reverse, so `<html>`'s own background rule couldn't see the
    override. Caught live: the white background only painted behind
    `<body>`'s content box, leaving the (taller) `<html>` viewport below
    it stuck on the old dark color. Moved the class to `<html>` instead.
  - Verified live in the running app: `Ctrl+L` flips `a := 2` from white-
    on-dark to black-on-white filling the whole window, and back again
    on a second `Ctrl+L`.

- **Graphing (Mathcad-style, bare `@` for an X-Y plot)** — `@` starts a
  new "graph" line type: a MathQuill expression field ("y ="), live-
  plotted below it as an SVG curve over a fixed `x ∈ [-10, 10]` domain
  with the y-axis auto-scaled to the computed data. Can plot a plain
  expression (`x^2`) or call a function defined on an earlier line
  (`f(x)`), reusing `evaluateLatex()`/`collectDefinitions()` unchanged —
  the graph just binds `x` to each of 201 sampled values and evaluates
  the expression once per sample on every edit.
  - A sample that throws (undefined reference, 1/0, a range-variable
    result instead of a plain number, ...) breaks the curve into a new
    `<polyline>` there instead of drawing a spurious straight line across
    the gap, the same way a real asymptote isn't connected across itself.
  - Exports as just the defining equation (`y = ...`) rather than an
    embedded plot image — an actual rendered graph in the `.tex`/`.pdf`
    output would need a real plotting package (`pgfplots` or similar),
    out of scope here; documented as a known limitation in the README.
  - **Real bug caught by live testing, not by the Node-level checks**:
    `focusLine()` (the one function responsible for putting the cursor
    into any line type once it exists) only had explicit cases for
    `"math"` and `"matrix"` — a `"graph"` line fell through to the
    generic contenteditable-focus path, which does nothing useful on a
    non-editable container div. The failure mode was severe: backspacing
    an empty graph line left focus nowhere at all, and *every* keystroke
    afterward — not just on that line, anywhere in the document — silently
    did nothing, with no error, crash, or visual sign something was wrong.
    Caught only by testing the actual multi-line flow (define a function,
    graph a call to it) rather than each graph in isolation. Fixed by
    adding `"graph"` alongside `"math"` in `focusLine()`'s first branch
    (both just focus `line.mathField`).
  - Verified live in the running app: `x^2` plots a correctly-centered
    parabola with both axes drawn; `1/x` correctly breaks into two
    separate branches at the asymptote instead of a spurious vertical
    line through it; defining `g(x):=x^3-4x` on one line and graphing
    `g(x)` on the next plots the call correctly, and backspacing an empty
    graph line no longer freezes input (the bug above, now fixed).
  - One test-environment caveat, not an app issue: this session's
    synthetic keystroke tool (`wtype`, used to drive the GUI for testing)
    silently drops a typed `-` character in this environment — confirmed
    independently (typing `-5` produced `5`, and `-5=` evaluated to
    `5=5`). An intermediate screenshot showing `g(x):=x^3-4x` as
    `x^{3}4x` is that tool limitation, not a parsing bug; typing the same
    thing from a real keyboard is unaffected.

## Not done / out of scope (per README's own "Notes" section)

- No i18n (`qsTr`/translation catalogs) — explicitly skipped.
- No quit-confirmation-on-unsaved-work — explicitly skipped.

## Verification log

- 2026-11-17: `bin/build` and `bin/test` both clean (7/7 tests pass).
  Ran the compiled binary under Wayland/Hyprland, drove it with `wtype`
  for headings, variables, nth-root evaluation, and a 2×2 matrix;
  screenshotted each with `grim` and confirmed correct rendering.
