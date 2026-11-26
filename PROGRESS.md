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

- **White-paper/black-ink toggle (`Ctrl+L`)** — flips between the
  white-background/black-text default and a dark-sheet alternative.
  Session-only (no settings storage in this app at all, so it's back to
  paper on next launch, same as window size/position).
  - `src/notebook.html`: the two appearances are `--bg`/`--ink` CSS custom
    properties, read by every color rule (directly, or via `currentColor`
    for things like the matrix brackets) rather than hardcoded colors, so
    `html.paper` overriding those two variables is the only place the
    look is actually defined; `togglePaperMode()` just toggles that class
    and is wired to `Ctrl+L` in the same global keydown listener as
    `Ctrl+S`/`Ctrl+Q`.
  - **2026-11-24: paper made the default**, at the user's request — the
    `<html>` tag now carries the `paper` class from page load, and
    `Ctrl+L` removes it for the dark sheet instead of adding it (the two
    were always just a CSS class presence/absence, so this is a one-line
    change, not a new code path). Verified live: a fresh launch shows
    white background/black ink with no keystroke needed; `Ctrl+L` still
    flips to the dark sheet and back.
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

- **3D surface plotting (same `@` graph line, auto-detected)** — the
  same graph expression field now plots a wireframe surface `z=f(x,y)`
  instead of a 2D curve, with no new shortcut: `renderGraph()` tries
  evaluating the expression with just `x` bound (exactly like the 2D
  path always has); if that throws specifically because `y` is the
  undefined reference, the expression must be using `y` as a second free
  variable, so it switches to 3D (binding both `x` and `y` across a
  15x15 grid, each always overriding any same-named `collectDefinitions()`
  entry, same rule as `x` already had). Anything else — `y` unused, or
  already defined by an earlier line like `y:=5` and used as that
  ordinary value — stays on the unchanged 2D path. The "y ="/"z =" label
  to the left of the field updates live in `renderGraph()` since the
  mode can't be known until the expression is actually evaluated.
  Rendered as an SVG wireframe (no 3D/graphing library, consistent with
  the rest of the project having no build step): a fixed isometric
  projection (x/y each tilt 30° off horizontal in opposite directions,
  z moves straight up the screen, scaled against the *computed* z-range
  the same way the 2D curve auto-scales its y-axis), with the projected
  footprint auto-fit/centered into the viewport from its own bounding
  box. A grid point that can't be evaluated (or evaluates to a
  non-finite number, e.g. a genuine 3D asymptote like `1/(x-y)` on the
  x=y diagonal) leaves a gap in the mesh lines through it rather than a
  spurious connection, checked per mesh segment in both the row and
  column directions.
  - Verified with a throwaway Node script
    (`detectGraphMode`/`renderGraph3D` copied verbatim out of
    `src/notebook.html`, with minimal `document`/`svg` stubs standing in
    for the DOM): a 2D-only expression (`x^2`) stays 2D; `x+y` with no
    prior `y` definition switches to 3D; `x+y` with an earlier `y:=5`
    line stays 2D and actually uses `y=5`; `y` left entirely unused
    (`sqrt(x)`) stays 2D; a defined single-variable function call (`f(x)`)
    stays 2D; a real paraboloid (`x^2+y^2`, every grid point finite)
    draws exactly the expected 2×15=30 unbroken mesh polylines; a genuine
    3D asymptote (`1/(x-y)`) breaks mesh lines at the x=y diagonal
    without throwing, into more than 30 polylines, none degenerate; and
    an expression where *every* grid point fails (`x+y+z`, `z` never
    defined) renders nothing without crashing. 43/43 assertions passed.
  - Verified live in the running app: typing `@x^2+y^2` renders a
    correctly-labeled (`z =`) wireframe paraboloid (bowl-shaped mesh,
    both grid directions visible). Re-ran the existing 2D cases
    afterward as a regression check — `@x^2` still plots the same
    parabola and `@1/x` still breaks into two branches at the asymptote,
    both unchanged. Separately confirmed live: `y:=5` on one line then
    `@x+y` on the next stays a 2D straight line (`y =` label) using the
    defined value (renders as `y=x+5`, matching the earlier Node check);
    and backspacing an empty graph line placed immediately after an
    existing graph line still correctly lands focus back in that
    previous line (the `focusLine()` "graph" case from the original
    feature still covers this; no regression there from adding 3D mode).

- **Greek letters (`Ctrl+<letter>`/`Ctrl+Shift+<letter>`)** — inserts a
  lowercase/uppercase Greek glyph into whatever math field, graph
  expression, or matrix cell has focus (see README's "Greek letters"
  table for the full mapping), usable as a real variable name, function
  name, or function parameter, not just a typed symbol.
  - `src/notebook.html`: `evaluateLatex()`'s identifier model (previously
    "one Latin letter, full stop") is generalized by `matchIdentifierAt()`
    to also accept one Greek command token (`\beta`, `\Gamma`, ...);
    `parseAtom`'s variable/function-call branch and the implicit-
    multiplication lookahead in `parseTerm` both go through it now.
    `collectDefinitions()`'s variable/function-definition matching was
    rewritten from two regexes to hand-rolled parsing (`parseDefinitionLHS()`
    + the same `matchIdentifierAt()`) — a regex alternation built from a
    dynamic list of Greek command strings needs the backslash
    double-escaped to work as `RegExp` source, and the project's own style
    already favors hand-rolled parsing (`eat`/`peek`) over that kind of
    regex gymnastics.
  - **Deviation from the originally-sketched mapping**: dropped the
    planned `Ctrl+Shift+S`→Σ and `Ctrl+Shift+P`→Π (2 of the original 11
    uppercase slots). Reason, found while implementing rather than
    assumed up front: `\Sigma`/`\Pi` already mean something specific to
    `parseAtom` (the summation/product prefix — see the `$`/`#` feature
    below) checked unconditionally before anything looks at whether it's
    a defined variable. Keeping the mapping as planned would've made
    `\Sigma:=5` followed by `\Sigma+1` silently evaluate as "reduce `+1`
    over nothing" (`= 1`) instead of `6` — a wrong-answer bug, not a
    cosmetic one, since both readings parse successfully. Σ/Π already
    have working, Mathcad-faithful shortcuts for their one existing
    meaning; every other letter in the original mapping is unaffected.
  - **Also investigated and deliberately did *not* do**: the task
    description raised extending MathQuill's `autoOperatorNames` (via
    `MQ.config()`) to cover `sech`/`csch`/inverse-sec-csc-cot/inverse-
    hyperbolic names, on the assumption the shipped `mathquill.min.js`'s
    default list was missing them. Checked empirically instead of
    assumed: typed each name into a throwaway `MathField` in a headless
    Chromium (`chromium --headless=new --dump-dom`) running the exact
    `mathquill.min.js` in this repo, and read back `.latex()`. Every one
    of them is already auto-unitalicized out of the box (rendered via
    `\operatorname{sech}` etc. rather than a native `\sech`, but still
    upright, not italic) — MathQuill's own startup code expands its
    hardcoded `sin cos tan sec cosec csc cotan cot ctg` list into every
    arc-/h-/ar- combination at load time, which isn't visible just
    reading the literal strings in the minified source. Calling
    `MQ.config()` wouldn't have extended that table either way — its
    setter *replaces* the whole table with only the names given — so
    doing it would have risked silently breaking unrelated existing names
    (`gcf`, `hcf`, `proj`, `Pr`, ...) for zero benefit. No `MQ.config()`
    call was made; see the comment above `var MQ` in `src/notebook.html`.
  - Verified with a standalone Node script (pure evaluator functions
    copied verbatim out of `src/notebook.html`, same pattern as every
    prior feature here): `\beta:=5` then `\beta+1` → `6`; a Greek function
    parameter (`f(\beta):=\beta^2`, `f(4)` → `16`); a Greek function name
    with mixed Greek/Latin params; implicit multiplication by a Greek
    identifier (`2\beta`, `\beta\gamma`); `\alpha` (deliberately unmapped)
    and `\Sigma`/`\Pi` (deliberately excluded) both confirmed NOT
    recognized as identifiers, with `\Sigma i^2` over a range still
    working as the sum operator; regression-checked a plain single-letter
    variable and function definition still work unchanged. 47/47
    assertions passed (shared with the built-in-functions feature below,
    same test file).
  - Verified live in the running app: `Ctrl+B` on a math field inserts
    `β`; `β:=5` on one line then `Ctrl+B` again on the next, `+1=`, gives
    `β+1=6`; `Ctrl+G` and `Ctrl+Shift+D` inside a 2-cell matrix render
    `[γ  Δ]`; `Ctrl+Shift+S` followed by typing `test` produces the plain
    word "test" (confirming Σ was correctly *not* inserted); the `$`
    sum-over-range shortcut (`Σk² = 14` over `k:=0…3`) still works
    unchanged. Also confirmed the Greek-letter shortcut only fires inside
    an actual math/graph/matrix-cell field — on a brand-new pending line
    it's a no-op.
  - **Correction, found on a later re-check, since fixed**: the fresh-
    pending-line no-op above was originally written off as "by design,
    the same as every other field-only shortcut." That was wrong — a
    plain-character shortcut (`\`, `;`, `$`, `#`, ...) *does* work as
    literally the first keystroke on a brand-new line, via a buffered-
    replay path (`wirePendingInput()` → `convertToMath()` →
    `insertChar()`) that a `Ctrl+<letter>` combo never enters, because
    holding Ctrl suppresses the browser's normal text insertion that
    path depends on — a gap specific to Greek letters (and any future
    modifier-key shortcut), not a shared property of every shortcut.
    **Fix**: `createPendingLine()` and `revertToPending()` now stamp
    `el._line = line` on the pending `<div>` (mirroring the back-
    reference math/matrix elements already carried); the Greek-letter
    block in the global keydown listener, when `resolveMathField()`
    finds no field yet, checks `e.target.closest(".pending")._line` for
    a still-pending line and, if found, calls `convertToMath(line, "")`
    to convert it right there before inserting the Greek command into
    the field that conversion just created. Verified live: `Ctrl+B`/
    `Ctrl+Shift+D` as the literal first keystroke on a fresh line now
    insert β/Δ and convert the line to math; re-verified unchanged:
    `\`/`;`/`$`/`@`/heading-marker as first keystroke, and `Ctrl+<letter>`
    mid-line in an already-converted field. `bin/build`/`bin/test` both
    clean (7/7).

- **Built-in named functions (`sin cos tan ...`, no definition needed)**
  — `sin cos tan sec csc cot` and their inverses (`arcsin arccos arctan
  arcsec arccsc arccot`), the hyperbolic versions (`sinh cosh tanh sech
  csch coth`) and their inverses (`arcsinh arccosh arctanh arcsech
  arccsch arccoth`), plus `ln` (natural log → `Math.log`), `log` (base 10
  → `Math.log10`), and `exp`, all evaluate directly — no `name:=...`
  definition line needed first, unlike every other function this app
  supports. MathQuill already renders every one of these upright the
  moment it's typed (confirmed empirically, see the Greek-letters entry
  above for why no `autoOperatorNames` change was needed).
  - `src/notebook.html`: two new lookup tables in `evaluateLatex()` —
    `NATIVE_BUILTINS` (`[latex command, JS function]` pairs, e.g.
    `["\sinh", Math.sinh]`, checked by exact command string) and
    `WRAPPED_BUILTINS` (`name -> JS function`, checked by the name inside
    a literal `\operatorname{...}`) — cover the two different ways
    MathQuill actually renders a recognized name (confirmed by the same
    headless-Chromium probe as above: `\sin\left(2\right)` for natively-
    LaTeX names, `\operatorname{sech}\left(2\right)` for the rest).
    `parseAtom` tries both tables (and `\operatorname{` unwrapping) before
    the generic identifier branch; `parseTerm`'s implicit-multiplication
    lookahead was extended the same way (plus `\operatorname{` and the
    Greek-command check above) so `2\sin\left(x\right)` parses correctly.
    `NATIVE_BUILTINS` is ordered longest-name-first within any group that
    shares a prefix (`sinh` before `sin`, `cosh` before `cos`, etc.) so
    greedy matching doesn't swallow `sinh(` as `sin` plus a dangling `h`.
    Every call goes through a shared `callBuiltin()` helper that in turn
    uses `broadcastUnary()`, so range broadcasting (and everything else
    that already works for `\sqrt`/`\frac`) works for these too, free.
    Inverse sec/csc/cot use the standard reciprocal identities
    (`arcsec(x) = acos(1/x)`, etc.).
  - **Deliberately not supported**: the shorter `a`-prefixed spelling
    some calculators use instead of `arc...` (`asin`, `asinh`, ...).
    Checked empirically (same headless-Chromium probe): MathQuill never
    auto-recognizes `asin` as one token at all — typing it live actually
    produces a separate italic variable `a` immediately followed by a
    recognized `\sin`, i.e. `a\sin\left(...\right)`, meaning "a times
    sin(...)" — an already-meaningful, different expression. Special-
    casing the evaluator to reinterpret that specific token sequence as
    arcsine would silently break the legitimate case where `a` is an
    actual defined variable.
  - Verified with the same standalone Node script as the Greek-letters
    feature above (47/47 total assertions): every named function on a
    plain scalar (`sin(0)=0`, `coth(1)`, `ln(1)=0`, `log(100)=2`,
    `exp(0)=1`, ...); broadcasting over a range variable (`sin(i)` and
    `ln(i)` over `i:=0…3`/`1…3`, and the `\operatorname{}`-wrapped
    `sech(i)`); inverse round-trips (`arcsin(sin(0.5))`,
    `arcsinh(sinh(0.5))`, `arcsec(sec(0.5))`, `arccot(cot(0.5))`,
    `arccsch(csch(0.5))`, `arccoth(coth(0.5))`, all ≈ `0.5`); the
    `sinh`/`sin` (and `cosh`/`cos`, `tanh`/`tan`, `coth`/`cot`) prefix
    case specifically, to catch the greedy-matching bug the ordering
    above exists to prevent; implicit multiplication by a builtin call
    (`2\cos(0)=2`); and the `a\sin(0)` "not arcsine" case from both
    directions (throws when `a` is undefined, evaluates as `a*sin(0)`
    when it's defined).
  - Verified live in the running app: `sin(0)=0` and `ln(1)=0`, both
    rendering the upright native `sin`/`ln` (unchanged from before this
    feature); `sin(k)` over a range `k:=0…3` gives
    `0, 0.841471, 0.909297, 0.14112`; `sech(0)=1`, rendering upright via
    `\operatorname{sech}` (confirming no `autoOperatorNames` regression);
    `arcsin(sin(0.5))=0.5`; and a `@sin(x)` graph (same "y=" expression
    field the existing graphing feature uses, which calls `evaluateLatex()`
    unchanged) plots a correct, unbroken sine curve — a regression check
    that the builtin-function plumbing didn't disturb graphing.

- **Matrix functions & equation solver** — matrices are now real,
  nameable values, not just standalone typeset widgets, plus
  `transpose`/`det`/`trace`/`inverse` and a `solve()` for both a single
  equation and a linear system.
  - **Matrices as values**: `A:[` on a math line showing exactly `A:=`
    (nothing else typed yet) names the matrix that follows — same
    spirit as `A:3` becoming `A := 3`. Still `line.type === "matrix"`
    (just with a new `line.name`), so every existing per-type
    dispatcher needed no changes. `collectDefinitions()` reads a named
    matrix line's grid of cells into a tagged value,
    `{ __matrix: true, rows: [[...]] }` — tagged rather than a bare
    array specifically so `broadcast()`/`broadcastUnary()`'s
    `Array.isArray()` checks (which mean "this is a range") never
    confuse the two; a matrix reaching either of those (`A+B`, `sin(A)`)
    throws a clear error rather than misbehaving silently. An unnamed
    matrix (the original bare `[` on a pending line) is unchanged —
    still just a typeset widget, not a value.
  - **`transpose` and `inverse` are `A^T`/`A^-1`, not function calls**
    — a deliberate pivot, not the original plan. MathQuill's
    `autoOperatorNames` table (what makes `sin(`/`det(` render upright
    instead of as separate italic letters) is a closure-internal table
    with no reachable reference to extend in place — `MQ.config()`
    *replaces* the whole table rather than adding to it (same
    conclusion the sech/csch investigation reached earlier in this
    file), and "transpose"/"inverse" aren't in it by default. Rather
    than accept that gap, `parsePow()` special-cases a matrix base:
    `A^T` → `transposeMatrix()`, `A^-1` → `inverseMatrix()` (Gauss-
    Jordan with partial pivoting) — this is also literally how Mathcad
    itself displays both operations (real toolbar operators, not typed
    words), so it's a better match for the app's own conventions, not
    just a workaround.
  - **`det`/`trace` are function calls.** `\det` *is* in MathQuill's
    native table (confirmed empirically, same standard as every other
    `autoOperatorNames` claim in this file), so `det(A)` renders
    upright for free. `trace` is not — typing it renders as 5 separate
    italic single-letter variables with no distinguishing mark, same
    underlying risk as the existing `asin` note above. Matched as a
    literal substring in `parseAtom`, gated to fire only when
    immediately followed by `(`, which was judged a much lower
    collision risk than `asin` (needs the exact letters `t-r-a-c-e` as
    adjacent variables *and* an immediately-following call-shaped
    paren). Cosmetic gap: `trace(A)` renders italicized, unlike `det`.
  - **A matrix-valued result renders as a real typeset matrix, not a
    flat number list** — but not via the originally-planned mechanism.
    The plan was to write `matrixLatex()`'s `\begin{bmatrix}...
    \end{bmatrix}` string into the field via `MathField.write()`;
    checked empirically before building anything (a throwaway
    `MathField`, `write()` with that exact string, read `.latex()`
    back) and found this bundled MathQuill build has **no bmatrix/array
    environment support at all** — `write()` with that string silently
    leaves the field empty. `matrixLatex()` itself is still correct and
    unchanged — it's only ever handed to the C++ `Backend` for export,
    which has a real LaTeX engine behind it, not MathQuill. The actual
    mechanism: `appendMatrixResult()` builds a small read-only
    bracket+grid widget (the same CSS an editable matrix line uses,
    with `MQ.StaticMath` cells instead of `MQ.MathField`) appended as a
    DOM sibling of the line's own field whenever a result is a matrix
    value; a field `edit` handler clears the stale widget on further
    edits to that line.
  - **`solve(expr, x)` / `solve(A, b)`** — one call, dispatching on
    whether the second argument is already a matrix-bound variable.
    Neither `solve` nor `trace` are in MathQuill's native name table
    either, matched the same literal-substring way. Single equation:
    Newton's method (`newtonSolve()`, fixed starting guess of 1 — not
    0, which trivially zeroes the derivative of any pure power and
    stalls on step one), chosen over bisection because bisection needs
    a bracketing interval this one-argument-per-side call shape has no
    room to supply. Known limitation inherent to Newton's method, not
    this implementation: returns whichever root the fixed guess
    converges to, not "the" root of an equation that may have several.
    System: `gaussianSolveSystem()` reduces `[A | b]` straight to
    `[I | x]`, reusing the same `gaussJordanEliminate()` helper
    `inverse()` is built on.
  - **A real, pre-existing, non-matrix-specific bug found and fixed
    along the way**: `insertChar()`'s `"="` handler evaluated the
    field's *entire* current LaTeX, including any `name:=` prefix,
    rather than just the RHS — so defining and displaying a value on
    one line (`x:=5=`) had always been broken (nothing had exercised
    that combination before; every existing verified example defines
    on one line and evaluates on the next). Fixed by applying the same
    `name:=` stripping `collectDefinitions()`'s `parseDefinitionLHS()`
    already does, inside `insertChar()`'s `"="` branch too.
  - Verified with Node-level logic tests against the pure matrix/solver
    functions (transpose, det, trace, inverse and its singular-matrix
    throw, `solve(x^2-4,x)=2`, a linear system, matrix-arithmetic safety
    guards throwing instead of misbehaving) and live in both the
    compiled app and a plain-Chrome-hosted copy of `notebook.html`
    (`bin/build`/`bin/test` clean, 7/7): `a:=[2,1;1,3]` then
    `det(a)=5`, `trace(a)=5`, `a^-1=` renders
    `[[0.6,-0.2],[-0.2,0.4]]`, `b:=a^T=` renders the transpose inline
    on a named-definition line (the combination the "=" prefix bug
    above was found through), `solve(x^2-4,x)=2`, and
    `solve(m,b)` for `m:=[2,1;1,3]`, `b:=[5;10]` → `[1;3]` (2·1+1·3=5,
    1·1+3·3=10 ✓). Statistical functions (`mean`/`median`/`variance`/
    `stdev`) shipped separately — see the entry right below.

- **Statistical functions** — `mean`, `median`, `variance`, `stdev`,
  each accepting either a range (already a flat array) or a named
  matrix value (flattened row-major into one list via
  `flattenCollection()`), returning a scalar.
  - **Variance/stdev use the population convention** (divide by n, not
    n-1): this evaluator has no notion of "a sample drawn from a larger
    unobserved population" — a range or matrix the user hands in is the
    actual, complete dataset on the sheet, so dividing by n ("the
    spread of exactly this data") is the more faithful reading. Matches
    Python's `statistics.pvariance`/`pstdev` default. Only one
    convention was built, matching this item's "small" scoping in
    `TODO.md`.
  - **Not in MathQuill's native operator-name table** (checked
    empirically, same method as every other such claim in this file) —
    all four use the same literal-substring-gated-on-`(` pattern as
    `trace`/`solve` from the matrix work above, with the same
    collision-risk reasoning.
  - A gotcha hit and fixed while checking the above empirically, now in
    `.claude/agents/frontend.md`'s lessons: the project's standard
    throwaway-headless-Chromium MathQuill check silently reports every
    name (even natively-recognized ones) as "unrecognized" if the test
    page loads `mathquill.min.js` without `jquery.min.js` first — this
    bundled MathQuill build depends on jQuery and throws during its own
    setup if `$` isn't defined yet, and `--dump-dom` doesn't surface
    that error on its own.
  - Verified with a Node-level check of the pure functions (mean/
    median/variance/stdev over a plain array, an even-length list, a
    messy unsorted/negative list cross-checked against an independently
    computed variance, a flattened 2×3 and 3×2 matrix, and error cases
    on an empty/non-collection argument) and live in the running app
    (`bin/build`/`bin/test` clean, 7/7): `i:=0…4` then `mean(i)=2`,
    `median(i)=2`, `variance(i)=2`, `stdev(i)=1.414214`; a named 2×3
    matrix gives `mean=3.5`, `median=3.5`, `variance=2.916667`,
    `stdev=1.707825`, matching independently-recomputed expectations;
    `mean(5)` (a bare scalar) leaves just `=` rather than crashing; and
    `trace` on a non-square matrix still correctly fails, confirming
    the new dispatch checks didn't disturb existing `trace`/`solve`/
    `det` ordering.

- **Inline code-completion for built-in function names** — an IDE-style
  autocomplete dropdown anchored at the live text cursor in any math-
  line/matrix-cell/graph-expression field. Typing 2+ letters that
  prefix-match a built-in function name (`sin`, `det`, `trace`, `solve`,
  `mean`, ... — see below) shows a dropdown; Up/Down moves a highlight,
  Enter/Tab finishes typing the match (plus the following `(`), Escape
  dismisses leaving the typed text untouched.
  - **Two design pivots before this shape was settled** (full history
    in the git log/conversation, condensed here): the user first asked
    for "a clever key combination" to surface "all the available
    commands and functions" → researched command-palette UX (VS Code/
    Linear/Vercel/Superhuman-style Cmd/Ctrl+K), designed as an `F1`-
    triggered searchable *reference* overlay (confirmed non-executing,
    specifically to avoid a generic command-dispatch layer) → the user
    then asked for it to be "code completion like" instead, which
    replaced the overlay idea entirely with inline, cursor-anchored
    autocomplete. Worth remembering: "inserts on select" here is still
    *not* the executable-command-palette risk that was deliberately
    avoided earlier — completion only ever finishes typing text already
    started, through the same `typedText()`/`.cmd()` calls every other
    shortcut already uses, never a new generic action-dispatch layer.
  - **Candidate list built from the evaluator's own data, not a
    separate hand-maintained list**: every `NATIVE_BUILTINS` entry
    (`sinh`/`sin`/.../`ln`/`log`/`exp`), `det` (native too, but its own
    `parseAtom()` branch rather than living in that table), every
    `WRAPPED_BUILTINS` key, and the literal-substring-matched names
    (`trace`/`solve`/`mean`/`median`/`variance`/`stdev`) — 34 names
    total. `transpose`/`inverse` are deliberately absent (they're
    `^T`/`^-1` operators on a matrix base, not typed names — nothing to
    complete). Greek-letter-by-spelled-name completion (e.g. typing
    "alpha") was deliberately scoped out too: that action would need to
    *delete* the typed prefix and substitute a command, a different and
    riskier mechanism than "finish typing this string," and Greek
    letters already have a fast one-keystroke shortcut once known.
  - **Match ordering matters**: sorted shortest-first (ties
    alphabetical), specifically so typing exactly `sin` highlights `sin`
    itself rather than `sinh` — `NATIVE_BUILTINS`' own array order lists
    `sinh` before `sin` (for the evaluator's own greedy-match parsing
    reasons), which would otherwise make `sinh` the default Enter/Tab
    target right after typing a complete, different, valid name.
  - **Cursor-anchoring question** (left open in the design, now
    resolved): MathQuill's live cursor is a real DOM element
    (`.mq-cursor`, confirmed empirically) with a normal
    `getBoundingClientRect()` in every field shape this app creates
    (math-line, graph-line expression field, matrix-cell) — all three
    are the same `MQ.MathField` constructor with a different container,
    so one `field.el().querySelector(".mq-cursor")` call needed no
    per-shape branching at all.
  - **A timing gap generalized from the Greek-letter fix**: the very
    first keystroke on a brand-new pending line isn't seen by the
    completion tracker either, for the identical reason the Greek-
    letter shortcut hit this on 2026-11-24 (the field doesn't exist
    until the "input" event fires, after the capture-phase keydown has
    already run) — e.g. typing "det(" as literally the first thing on a
    fresh line tracked only "et". Fixed the same way: `convertToMath()`
    seeds completion tracking from the buffered text it's already
    replaying, when that text is a run of lowercase letters.
  - Verified with a Node-level check of the candidate-list/match/sort
    logic, and live against the real `src/notebook.html` (served over
    HTTP, driven with real synthetic keyboard events — both via the
    implementing session's headless-Chromium+CDP harness and, in a
    separate independent check from the manager session, a visible
    Chromium window via `claude-in-chrome`): `bin/build`/`bin/test`
    clean, 7/7. Typing "de" in a math line shows "det", positioned at
    the cursor; Enter inserts `\det\left(\right)`. Typing "mea" in a
    matrix cell shows "mean"; typing "tr" shows "trace". Typing exactly
    "sin" then Enter correctly inserts `\sin\left(\right)`, not
    `sinh(...)`. Escape dismisses cleanly, leaving the field fully
    usable (confirmed by continuing to type normally afterward).
    Typing "sin(0)=" entirely by hand, never touching the dropdown,
    still evaluates to `0` exactly as before. `Ctrl+B` (Greek letters,
    sharing the same keydown listener) still inserts β correctly as a
    fresh line's first keystroke.
  - **An environment incident worth recording, not an app bug**: the
    implementing session's first live-testing attempt used `wtype`
    against the compiled Qt app and, despite reporting success,
    Hyprland's window focus silently stayed on a different window —
    sending a handful of stray keystrokes into the manager session's
    own terminal input instead of the livesheet window. Caught quickly
    and had no lasting effect (confirmed: nothing was sent or executed
    as a result), but see `.claude/agents/frontend.md`'s lessons for the
    full writeup and the safer alternative this pushed verification
    toward.

- **Symbolic derivatives** (`TODO.md`'s "symbolic calculus" item,
  phase 1 of 2 — indefinite integrals are phase 2, still queued, see
  `TODO.md`) — a line typed in Leibniz notation, `d/dx(expr)` (typed
  naturally: `d`, `/`, `d`, `<var>`, Right-arrow out of the denominator,
  then the expression), evaluates to the symbolic derivative *formula*
  of `expr` with respect to `<var>`, not a number at a point.
  - **Why this needed new infrastructure, not just a new builtin**:
    every line of `evaluateLatex()` goes straight from LaTeX to a
    number — there was no expression-tree anywhere in the codebase, and
    a symbolic result needs one (apply a rule to a node, get back
    another node, not a value). Built as a brand-new, purely **additive**
    layer in `src/notebook.html` (confirmed via `git diff | grep '^-'`
    showing a single non-logic comment line removed, nothing else) —
    `evaluateLatex()`/`collectDefinitions()` were not touched at all,
    so every existing math/matrix/graph line's numeric evaluation is
    unaffected by construction, not just by testing.
  - **Node shape** (for integrals, phase 2, to reuse — don't redesign
    it): `{type:"num",value}`, `{type:"var",name}`, `{type:"neg",arg}`,
    `{type:"add"|"sub"|"mul"|"div", left, right}`,
    `{type:"pow", base, exp}` (exp is itself a node), `{type:"call",
    name, arg}` (`name` one of `sin`/`cos`/`tan`/`exp`/`ln`/`log`).
    Every node is immutable — every function returns a brand-new node,
    never mutates in place, so the same subtree can safely appear twice
    in one result (the exponential rule reuses the original `b^f`
    subtree verbatim).
  - **Pipeline**, each stage its own reusable top-level function:
    `parseLatexToTree(latex)` — a full separate recursive-descent
    parser mirroring `evaluateLatex()`'s own grammar/precedence, but
    narrower (no ranges/Σ/Π/matrices/user functions/det/trace/solve/
    stats, and it never touches `vars`/`funcs` — every identifier is
    just a name until differentiation decides what to do with it) →
    `differentiate(tree, varName)` → `simplifyTree(tree)` (a small
    identity-cleanup pass — folds `+0`/`*1`/`^0`/`^1`/double-negation/
    constant-constant arithmetic, deliberately NOT a general simplifier
    per the agreed scope, so e.g. the quotient rule on `x/(x+1)` renders
    as the correct-but-unreduced `\frac{x+1-x}{(x+1)^2}`, not `\frac{1}
    {(x+1)^2}` — not a bug) → `treeToLatex(tree)` (precedence-aware
    renderer back to display LaTeX).
  - **Rules implemented** (the agreed v1 scope — not a general CAS):
    constant → `0`, the diff variable → `1`, any other variable → `0`
    (another variable is just as "constant" to a given differentiation
    as a literal number — `containsVar()` checks this correctly, not a
    literal-number check), sum/difference linearity, product rule,
    quotient rule, power rule for a constant exponent, a general
    exponential rule for a constant base (`d/dx(b^f) = b^f·ln(b)·f'`,
    covering `exp(f)`/`e^f` the same way as any other base — note `e`
    is just an ordinary `var` node to this layer, not a recognized
    constant, so `ln(e)` does NOT simplify away; `d/dx(e^{3x})` renders
    as the correct-but-unreduced `e^{3x}\cdot\ln(e)\cdot3`, confirmed
    live, not a bug), `sin`/`cos`/`tan` (`tan` returns `f'/\cos(f)^2`
    rather than introducing a `sec` node, since `sec` isn't in this
    layer's vocabulary), `ln` (`f'/f`), `log` (base-10, `f'/(f\cdot
    \ln10)`, matching this codebase's own existing `\log` → `Math.log10`
    convention elsewhere, not the calculus-textbook natural-log
    convention). The chain rule falls out automatically everywhere via
    "multiply by the inner derivative" at every rule, rather than
    needing separate handling. Throws (falls back to "can't evaluate
    yet" UX) on `f(x)^g(x)` (both base and exponent depend on the diff
    variable) and on any function name outside the six supported — a
    hyperbolic function (`\sinh` etc.) inside the expression would
    mis-parse as `\sin`+dangling `h` rather than being recognized or
    rejected cleanly; a known, documented gap, not fixed since
    hyperbolic derivatives aren't in scope.
  - **Trigger syntax, confirmed live, not assumed**: a typed `d`, `/`,
    `d`, `<var>` produces exactly `\frac{d}{dx}` in MathQuill (the
    denominator's `d` and the variable are two adjacent single-letter
    nodes, no separator) — confirmed via the DevTools-Protocol fallback
    below, not guessed from the LaTeX spec. `matchDerivativeTrigger()`
    recognizes this shape specifically (numerator trims to exactly `d`,
    denominator is exactly `d` + one identifier token and nothing else)
    so it never collides with an ordinary typed fraction/division;
    checked first in `evaluateAndAppend()`, before the numeric path
    runs at all, falling through (returns `null`) to the ordinary
    numeric evaluator for anything that doesn't match. Works for a
    Greek diff variable too (`d/d\beta`), confirmed live.
  - **Display**: a new, deliberately generic `appendFormulaResult(line,
    latex)` — any LaTeX string in, no derivative-specific assumptions —
    renders a read-only `MQ.StaticMath` as a DOM sibling of the line's
    own field, same spirit/placement as the existing
    `appendMatrixResult()`. New `.result-formula` CSS class, same
    pattern as `.result-matrix`. `convertToMath()`'s edit handler now
    also clears a stale `resultFormulaEl` on any edit, mirroring the
    existing `resultMatrixEl` cleanup. Built specifically so the
    upcoming integral feature (`TODO.md`) can reuse this exact function
    rather than needing its own result-display mechanism. Known,
    deliberate limitation carried over from the matrix-result pattern:
    this is DOM-only — `collectDocument()`'s LaTeX export won't include
    an appended formula result, same pre-existing limitation matrix
    results already have, not a new regression.
  - One inline code comment (on the exponential rule) was found and
    fixed during review: it originally claimed `ln(e)` "simplifies to 1
    in `simplifyTree()` below," which contradicts both the actual code
    (`simplifyTree()`'s `call` case has no such special case) and the
    live-tested output above — corrected to accurately describe the
    real, documented behavior instead.
  - Verified with a Node-level test of the parse/differentiate/
    simplify/render pipeline (29 assertions covering every rule above,
    including the required chain-rule composition `d/dx\sin(x^2)`, and
    both positive/negative `matchDerivativeTrigger()` cases), and live
    against the real `src/notebook.html` (headless Chromium over the
    DevTools Protocol via Node's native `WebSocket` — `claude-in-chrome`
    wasn't connected in this environment; recipe documented in
    `.claude/agents/frontend.md`'s 2026-11-25 entry). `bin/build`/
    `bin/test` clean, 7/7, confirmed again after the comment fix above.
    Live: `d/dx(x^2)` → `2\cdot x`; `d/dx\sin(x^2)` → `\cos(x^2)\cdot2
    \cdot x`; `d/dx(x^3+2x)` → `3x^2+2`; backspacing after a result
    clears the stale formula widget correctly. Regression spot-check,
    same Chromium session: `2+3=` → `5`, `\sin(0)=` → `0`, `x:=5` then
    `x+1=` → `6`, and `@\sin(x)` still converts to a graph and renders
    its curve — all unaffected, confirming the new derivative call site
    is a genuinely separate branch that never runs for an ordinary line.

- **Symbolic indefinite integration** (phase 2 of symbolic calculus,
  completing the `TODO.md` item) — a line typed as `\int(expr)dx`
  evaluates to the symbolic antiderivative of `expr`, reusing phase 1's
  AST layer (`parseLatexToTree`/`containsVar`/`simplifyTree`/
  `treeToLatex`/`appendFormulaResult`) as-is, with zero changes to
  `evaluateLatex()`/`collectDefinitions()` or the derivative code path
  — purely additive, confirmed via `git diff | grep '^-'` showing only
  comment-text removals.
  - **Trigger, and why it isn't bare `\int f(x)\,dx`**: `Ctrl+Shift+I`
    inserts `\int` immediately followed by an auto-paired `(`, so the
    typed/rendered shape is `\int\left(f(x)\right)dx`. Confirmed live
    that MathQuill's `\int` is a bare, slot-less symbol in this app's
    bundled build (not a `\Sigma`/`\Pi`-style template with sub/
    superscript boxes — there's no "leave the bounds empty" option for
    an indefinite integral, there are no bounds slots at all), and that
    a bare `\int` with no block boundary gets swallowed into a
    numerator by MathQuill's own auto-fraction grab-back the moment the
    integrand contains a `/` (confirmed: `\int` then `1/x` with nothing
    between produces `\frac{\int1}{x}`, not `\int\frac{1}{x}`) — the
    forced paren wrapper exists specifically to give the integrand a
    block boundary that grab-back respects, which is what makes the
    required `n=-1` case (`∫1/x dx`) typeable at all, not a style
    choice. `matchIntegralTrigger()` detects the shape purely textually
    (starts with `\int`, ends with literal `d` + one identifier token),
    same "return null to fall through to the numeric path" convention
    `matchDerivativeTrigger()` already established.
  - **Why `Ctrl+Shift+I` and not `Ctrl+I`**: Mathcad's own real
    keyboard convention (PTC docs, web-searched) is version-dependent —
    legacy Mathcad uses `Ctrl+I` for the indefinite integral, later
    Prime-era Mathcad uses `Ctrl+Shift+I`. `Ctrl+I` is already claimed
    in this app by the Greek-letter feature (`\iota`), so
    `Ctrl+Shift+I` was used — free, and itself a genuine Mathcad
    convention, not an arbitrary pick. `Ctrl+Shift+I` is also
    Chromium's own "open DevTools" shortcut, handled with the same
    `preventDefault()`/`stopImmediatePropagation()` override this app's
    `Ctrl+S`/`Ctrl+Q` already use.
  - **Rules implemented** (same agreed-scope spirit as phase 1, not a
    general CAS): power rule including the `n=-1` case (`→ ln(x)`,
    handled separately from the general power formula to avoid
    dividing by zero), `sin`→`-cos`, `cos`→`sin`, `exp(f)`/`e^f`→itself,
    a constant base `b^x`→`b^x/\ln(b)`, linearity (`∫(f+g) = ∫f+∫g`),
    and constant-factor pull-out (`∫k\cdot f = k\cdot∫f`). **"Simple
    compositions" is scoped specifically to linear substitution**
    (`∫f(ax+b)dx = F(ax+b)/a`, via a new `linearCoeffs()` helper that
    structurally detects "linear in the integration variable" the same
    way phase 1's rules already distinguish "constant w.r.t. this
    variable" from "literal number" — not general u-substitution
    pattern-matching, which was deliberately ruled out as an unbounded
    problem). `\tan`/`\ln`/`\log` are explicitly out of scope (not just
    unimplemented) and a non-linear composition (e.g. `∫2x\sin(x^2)dx`,
    a product of two factors both depending on the integration
    variable) fails gracefully rather than guessing — confirmed live
    that an unsupported sub-term anywhere (even nested inside a sum)
    fails the *whole* integral rather than partially integrating and
    leaving a dangling piece, by simply letting the exception propagate
    uncaught up through the recursive rules to the same top-level
    try/catch the derivative trigger already uses.
  - Verified with a Node-level test (29 assertions: every rule above,
    `n=-1`, two linear-substitution cases, linearity/pull-out, the
    "other variable treated as constant" case, and the graceful-
    failure cases for `tan`/`ln`/`log` — standalone and nested in a sum
    — and the non-linear-composition case), and live against the real
    `src/notebook.html` (headless Chromium over the DevTools Protocol,
    `claude-in-chrome` wasn't connected in this environment). `bin/
    build`/`bin/test` clean, 7/7. Live: `Ctrl+Shift+I` on a brand-new
    pending line inserts `\int\left(\right)` correctly (confirming the
    same "fresh pending line" fallback phase 1 needed was re-applied
    for this new shortcut too); `\int(1/x)dx` → `\ln(x)`; `\int\sin(3x)
    dx` → `-\cos(3x)/3`; `\int e^{2x}dx` → `e^{2x}/2`; `\int\tan(x)dx`
    and `\int2x\sin(x^2)dx` both leave the line unevaluated (graceful
    failure, no result appended); `Ctrl+Shift+I` on a heading line is a
    safe no-op. Regression spot-check, same session: `3+4=` → `7` and
    `d/dx(x^2)=` → `2\cdot x` (phase 1) both still correct, confirming
    this phase never touched the numeric or derivative paths.

- **Program blocks** (`TODO.md`'s last and biggest backlog item — a
  real programming construct, styled close to math notation) — `{`
  starts a program block, either bare on its own line or as a
  function's body (`f(x):={`, mirroring how `[` already starts a named
  matrix). A program is a flat, keyboard-navigable sequence of
  statement rows: local assignments (`x\leftarrow expr`, typed via `:`
  inside a statement field — the program-scoped meaning of the same
  physical key that means worksheet-level `:=` on an ordinary math
  line, not a collision, since Mathcad itself genuinely uses a
  different symbol for the two), at most one `for`/`while` loop, at
  most one `if`/`otherwise`, and `break`/`continue`/`return` (inserted
  via `Ctrl+Shift+R`/`W`/`N`/`B`/`T`/`E` — see below for why these
  letters, none of which are Mathcad conventions). A program evaluates
  to its last statement's value unless an earlier `return` exits it
  first (Mathcad's own convention). One level of nesting only (the
  decided v1 scope): a loop's or `if`'s own body is plain assignment
  statements, enforced structurally (`insertStructural()` refuses to
  insert a second top-level loop/`if`, or any structural insert at a
  nested level at all), not just by convention.
  - **Reuses existing infrastructure rather than inventing a parallel
    mechanism**: a program-block function body is registered through
    the exact same `name(params):=rest` path `collectDefinitions()`
    already uses for an ordinary function (`funcs[name] = {params,
    program: line}` instead of `{params, body: latexString}`);
    `evaluateLatex()`'s function-call site gained exactly one new
    branch (`fn.program ? runProgram(...) : evaluateLatex(fn.body,
    ...)`) and is otherwise unchanged. A bare (non-function) program
    block evaluates in place the same way an ordinary math line does,
    through the same trigger-detection convention symbolic calculus's
    `matchDerivativeTrigger`/`matchIntegralTrigger` already
    established in `evaluateAndAppend()`.
  - **Comparison operators didn't exist before this** (`evaluateLatex()`
    was arithmetic-only) — added `<`/`>`/`=`/`\leq`/`\geq`/`\neq` as a
    new `parseComparisonExpr()` precedence level directly below
    `evaluateLatex()`'s entry point (binding looser than `+`/`-`,
    non-chaining, representing true/false as plain `1`/`0` — no new
    boolean type). Confirmed zero regression risk: no pre-existing
    line's LaTeX can ever contain a literal `=`/`<`/`>` character,
    since `insertChar()`'s own `"="` branch always captures a math
    line's text *before* that keystroke is inserted.
  - **Local assignment scope**: `runProgram()` copies the caller's
    `vars` into a fresh local object once, up front; every `←`
    assignment and `for`-loop binding writes only into that local
    copy, never the caller's own object — confirmed live that a
    program parameter/local variable shadows an outer sheet variable
    of the same name during execution and never leaks back out after.
  - **Control flow via thrown sentinels** (`{__programSignal:
    "break"|"continue"|"return", value?}`), caught by the enclosing
    loop (`break`/`continue`) or the top-level `runProgram()`
    (`return`) — consistent with how this evaluator already uses
    throw/catch for its own error propagation. A `break`/`continue`
    that escapes every enclosing loop (used outside one entirely) is a
    clear reported error, not a silent no-op or an uncaught exception
    reaching the UI.
  - **Infinite-loop safety**: both `for` and `while` are hard-capped at
    `PROGRAM_MAX_ITERATIONS = 100000`; exceeding it throws a clear
    error instead of hanging the `WebEngineView` with no way to cancel
    — Node-tested directly against a `while(1)` that never terminates.
  - **`for` iterates a range value or a flattened named matrix**
    (reusing `flattenCollection()`, the same helper `mean`/`median`/
    `variance`/`stdev` already use) — a bare scalar collection
    expression is rejected the same way those functions already reject
    one. Iterating one row/column of a matrix specifically is not
    supported (no existing representation of "one row of a matrix"
    anywhere in this file to build on) — a documented v1 limitation,
    not silently missing.
  - **Editor model**: a new `"program"` line type, added to every
    existing `line.type` dispatcher (`focusLine()`, `collectDocument()`/
    export, `collectDefinitions()`'s registration) rather than any
    being missed — this project's own hard-won lesson about exactly
    that failure mode (`.claude/agents/frontend.md`'s lessons). Visual
    grouping reuses the same CSS-border bracket technique the matrix
    lines already use (`.matrix-bracket-left`), not a stretched glyph
    or anything copied from a specific app's UI. Tab/Left/Right walk a
    freshly-computed flattened reading-order list across the whole
    nested structure; Enter (via MathQuill's own configurable
    `handlers.enter`, confirmed NOT reachable through a simulated
    `.keystroke("Enter")` — see below) adds a sibling statement in
    whichever list contains the focused field; Backspace on an empty
    plain statement row removes it, and emptying a bare program's only
    statement reverts the whole line to pending (mirrors the matrix
    line's own revert-to-pending behavior).
  - **Why the block-start/control-flow keys are what they are**:
    Mathcad has no bare-key convention for inserting a program block or
    its for/while/if/break/continue/return at all — its real
    Programming toolbar inserts every one of these by a toolbar click
    (PTC's own docs, web-searched, not guessed), so none of this app's
    choices here are a Mathcad convention the way `←`/`∈`/`:=`/`;` are.
    `{` (proposed in `TODO.md`, confirmed still unclaimed) starts a
    block. `Ctrl+Shift+R`/`W`/`N`/`B`/`T`/`E` insert
    for/while/if/break/continue/return — chosen only from letters free
    of every existing Ctrl/Ctrl+Shift binding (every bare `Ctrl+<letter>`
    is already claimed by Greek letters or this app's own `s`/`q`/`l`;
    `Ctrl+Shift+I` is already the integral trigger), so mnemonics are
    weak for a couple of them (documented inline, not silently picked):
    R for "for" (a for-loop iterates a Range), W for "while" (exact),
    N for "if" (I was taken), B for "break" (exact), T for "continue"
    ("conTinue"), E for "return" ("rEturn"). `for`/`while`/`if`/
    `otherwise`/`∈` render as static text labels, never typed into
    MathQuill at all — sidesteps `TODO.md`'s own open question about
    whether MathQuill's `autoOperatorNames` already covers these
    keywords, by simply never needing it to.
  - **A safety fix made during review, not by the implementing
    session**: the new `Ctrl+Shift+R`/`W`/`N`/`B`/`T`/`E` handler
    originally only called `preventDefault()`/`stopImmediatePropagation()`
    when a program field actually resolved from the event target —
    mirroring the existing (deliberate, documented) Greek-letter/
    integral-trigger pattern of letting the browser default through
    otherwise. That's an acceptable tradeoff for those older shortcuts
    (a missed one opens DevTools or does nothing), but `Ctrl+Shift+R`
    specifically collides with Chromium's own hard-reload shortcut, and
    this app has zero save/persistence — letting that through from a
    stray keystroke while focus was anywhere else (e.g. an ordinary
    line) would have silently wiped the entire in-progress notebook.
    Fixed to preventDefault/stopImmediatePropagation unconditionally
    for all six letters whenever the modifier combo matches, regardless
    of whether a program field resolves.
  - **Two real bugs found and fixed live, not caught by the Node-level
    tests alone**: (1) a program's own trailing/last statement — never
    forced to be a named assignment, per the "last statement's value"
    convention — originally threw "malformed assignment" unconditionally;
    fixed by falling back to evaluating the field as a bare expression
    when it contains no `\leftarrow` at all. (2) a brand-new `if`
    originally seeded a starter row only in `thenBody`, leaving
    `elseBody` permanently empty with no way to ever focus/add a row
    into it through the UI — fixed to seed both branches.
  - **A MathQuill finding worth remembering**: `.keystroke("Enter")`
    does **not** invoke a configured `handlers.enter` callback at all
    (confirmed even on an already-working ordinary math line, not just
    a program field) — `.keystroke("Left"/"Right")` do. Testing Enter
    live requires a real CDP `Input.dispatchKeyEvent` with
    `type:"keyDown"` (not `"rawKeyDown"`) plus
    `windowsVirtualKeyCode`/`nativeVirtualKeyCode:13` and
    `text`/`unmodifiedText:"\r"` — a JS-constructed `KeyboardEvent`
    can't fake `which`/`keyCode` (getter-only in modern Chromium),
    which is what this bundled MathQuill build's internal listener
    actually reads.
  - Verified with a Node-level test (comparisons, sequential
    assignment, shadowing/local-scope, a for-loop over a range,
    while+break, for+continue, early return via `if`, both `if`/
    `otherwise` branches, break-outside-loop error, the iteration-cap
    error, program-backed function-call dispatch, the empty-program
    error, and the bare-trailing-expression fix), and live against the
    real `src/notebook.html` (headless Chromium over the DevTools
    Protocol, `claude-in-chrome` wasn't connected in this environment).
    `bin/build`/`bin/test` clean, 7/7, re-confirmed again after the
    `Ctrl+Shift+R` safety fix above. Live: built a complete bare
    program (`s\leftarrow0`, `for i\in r: s\leftarrow s+i`, trailing
    `s`) through real `{`/Enter/`Ctrl+Shift+R`/`:`/`=` keystrokes →
    `s=10`; built a complete function-body program (`f(x):={if x>0:
    y\leftarrow1 otherwise y\leftarrow-1; y}`) the same way →
    `f(5)=1`, `f(-5)=-1`; Tab navigation across a for-loop's var/
    collection fields, break/continue/return insertion, and backspace-
    revert-to-pending on an emptied bare program all confirmed.
    Regression, same session: `2+3=` → `5`, `d/dx(x)=` and `\int(x)dx=`
    both still produce a formula result — unaffected.
  - **Backspace now deletes a whole pristine `for`/`while`/`if`
    structure**, closing the rough edge noted above: Backspace-when-
    empty on a structure's first header field (`for`'s "var",
    `while`/`if`'s "cond") removes the entire structure — but only when
    `isStructurePristine()` confirms nothing's been typed anywhere in
    it since insertion (every header field still empty, body/bodies
    still exactly the untouched starter statement). A non-pristine
    structure, or Backspace on a `for`'s second header field ("coll"),
    keeps the old behavior (move focus to the previous field) —
    deliberately scoped this way so a half-filled-in loop/if can never
    be silently discarded by one keystroke; this is the intended final
    shape, not a remaining gap. Live-verified over the CDP fallback
    (bare for/while/if each revert the whole line to pending when
    deleted as the program's only statement; a `for` inserted as a
    second top-level statement removes cleanly without disturbing the
    assign statement before it; typing into a structure's body/header
    before backspacing correctly blocks deletion; the "coll" field
    still only navigates, never deletes), including one pass with a
    genuine CDP `Input.dispatchKeyEvent` Backspace (not a direct
    function call) to confirm MathQuill's `deleteOutOf` wiring reaches
    the new logic end-to-end.
  - **Program-block nesting is now unrestricted**, closing the other
    rough edge noted above: any number of `for`/`while`/`if` structures
    can coexist at the same level, and any of them can nest inside
    another's body/branches, to any depth (previously: at most one
    `for`/`while` and one `if`, ever, both top-level only, bodies
    restricted to plain statements). The interpreter
    (`runStatement()`/`runStatementList()`), the field-navigation model
    (`programFieldList()`), statement lookup (`locateStatement()`), and
    rendering (`renderStatement()`'s `nestedBody()`) were all already
    written as plain unconditional recursion with no depth assumption —
    the v1 cap was enforced in exactly one place,
    `insertStructural()`'s guard clauses, so lifting it was a one-
    function change plus updating several comments that had asserted
    "only one level" as if it were still enforced elsewhere. Also
    confirmed the pristine-Backspace-deletes-a-structure behavior above
    generalizes to a nested structure with no extra code: deleting a
    nested, untouched structure removes just that one, leaving its
    parent and siblings untouched; a nested structure with real content
    still can't be deleted this way, same safety rule as at the top
    level. Live-verified (claude-in-chrome connected this session):
    nested a `while` inside a `for`'s body via real `Ctrl+Shift+R`/`W`
    keystrokes; ran two sequential top-level `for` loops end to end
    (`s←0; for i in r: s←s+i; for i in r: s←s+i; s` → `12`, confirming
    both actually execute, not just that the UI accepted the insert);
    inserted a second top-level `if`; built and ran a 2-level nested
    `for`-inside-`for` sum (`for i in r: for j in r: s←s+i·j` → `36`,
    i.e. `(1+2+3)²`); confirmed Backspace-deletes-pristine-structure on
    a nested structure both deletes-when-pristine and declines-when-
    not, exactly mirroring the top-level case above. One cosmetic note
    recorded in-code rather than fixed: `.program-body-nested`'s
    `opacity: 0.92` compounds multiplicatively per nesting level (≈0.85
    at 2 levels, ≈0.78 at 3) since each nested div is its own
    compositing layer — checked live at 3 levels and judged still
    clearly legible, so left as-is.
  - **Newly-documented v1 limitation, found while verifying the above,
    not introduced by it**: a `for`'s collection field only accepts a
    reference to an already-defined range variable (`i:=1..3` on an
    earlier line, then `for x in i`) — typing an inline range literal
    directly into the field (`for x in 1..10`) fails, because
    `evaluateLatex()` itself has no `\ldots`/range-literal handling at
    all; only `parseRangeDefinition()` (used solely by
    `collectDefinitions()` for a `name:=start..end` binding line) does.
    This is exactly the natural thing a user would try first, and fails
    with an unhelpful "trailing input" error rather than a clear
    message — worth revisiting (either teach `evaluateLatex()` to parse
    an inline range, or at least give `for`'s own error path a clearer
    message for this specific case) if real use of program blocks hits
    it, but not explicitly requested yet, so not queued as open work.
  - **General matrix indexing** — `M_{i,j}` (scalar element at row `i`,
    column `j`, 1-indexed) and `M_i` (the `i`-th element of a
    vector-shaped matrix, 1 row or 1 column; a clear error on a
    genuinely 2D matrix telling the user to use `M_{i,j}` instead), plus
    new `row(M, i)`/`col(M, j)` builtin functions returning a new 1×n
    row or n×1 column matrix value. This is the real fix for the
    `for`-row/column rough edge noted above: `for x in row(M, 2)` needed
    ZERO changes to `for`'s own runtime code, because `flattenCollection()`
    already flattens any matrix value row-major — `row()`/`col()` just
    had to hand it one. Empirically confirmed before writing any parsing
    code (throwaway MathField against the shipped `mathquill.min.js`,
    same standard as every other LaTeX-emission claim in this file): a
    typed `_` emits a bare, unbraced single character for exactly one
    plain character (`M_2`, `M_i`) — same "single character skips
    braces" rule `parsePow()`'s `^T` exponent handling already relies
    on, confirmed separately rather than assumed by analogy — and a
    braced `{...}` group for anything longer (multi-digit, a
    comma-separated pair, a Greek command, a negative number); an empty
    subscript renders as `M_{ }`, never a bare dangling `_`, so there's
    no reachable brace-less-and-empty gap in the parser. Neither `row(`
    nor `col(` is in MathQuill's native `autoOperatorNames` table (come
    back as plain typed letters, same as `trace`/`solve`/`mean`), so
    both use the same literal-substring-gated-on-"(" treatment as those.
    Index validation (`checkMatrixIndex()`): must be finite and within
    `1e-9` of an integer, then bounds-checked to `[1, max]`; a clear,
    label-specific error either way. A subscript after a plain scalar
    variable (not a matrix) is a clear error too, not silent
    misbehavior. Live-verified: a real 3×3 matrix and a real 1×3 vector
    built through the app's own DOM-builder functions; `M_{2,3}=`,
    `V_2=`, `row(M,2)=`/`col(M,2)=` (each rendering as a real
    `.result-matrix` via the existing, unchanged matrix-result display
    path) all correct; `M_1` on the 2D matrix throws the documented
    "use `M_{i,j}` instead" error; out-of-range and non-integer indices
    on both `M_{i,j}` and `row`/`col` all rejected cleanly; a real
    program-block statement tree (`s←0; for x in row(M,2): s←s+x; s`,
    built with the actual statement/field constructors, not simulated
    keystrokes) correctly summed row 2 (`[4,5,6]` → `15`) through the
    real, unmodified interpreter; `det`/`trace`/`^T`/`^-1`/plain-scalar
    regressions all unaffected.

## Not done / out of scope (per README's own "Notes" section)

- No i18n (`qsTr`/translation catalogs) — explicitly skipped.
- No quit-confirmation-on-unsaved-work — explicitly skipped.

## Verification log

- 2026-11-17: `bin/build` and `bin/test` both clean (7/7 tests pass).
  Ran the compiled binary under Wayland/Hyprland, drove it with `wtype`
  for headings, variables, nth-root evaluation, and a 2×2 matrix;
  screenshotted each with `grim` and confirmed correct rendering.
- 2026-11-17: 3D surface plotting added to the existing `@` graph line.
  `bin/build`/`bin/test` both clean (7/7). A standalone Node script
  (functions copied verbatim out of `src/notebook.html`) passed 43/43
  assertions on the 2D/3D detection rule and the 3D grid/projection/gap
  logic. Live in the running app: `@x^2+y^2` renders a correctly
  z-labeled wireframe paraboloid; the original `@x^2` and `@1/x` 2D
  cases, and the already-fixed empty-graph-line backspace/focus
  behavior, all re-verified unchanged; `y:=5` then `@x+y` confirmed to
  stay 2D and plot `y=x+5`.
- 2026-11-17: Greek letters (`Ctrl+<letter>`/`Ctrl+Shift+<letter>`) and
  built-in named functions (`sin`/`cos`/.../`ln`/`log`/`exp`, inverse and
  hyperbolic forms). `bin/build`/`bin/test` both clean (7/7). A standalone
  Node script (pure evaluator functions copied verbatim out of
  `src/notebook.html`) passed 47/47 assertions covering both features
  together (variable/function definitions and lookups, implicit
  multiplication, range broadcasting, inverse round-trips, the deliberate
  `\Sigma`/`\Pi`/`asin`-style omissions, and a regression check of the
  pre-existing single-Latin-letter path). Also used a headless Chromium
  (`chromium --headless=new --dump-dom`) running the actual
  `mathquill.min.js` shipped in this repo to confirm, empirically rather
  than by reading the minified source, exactly how MathQuill renders each
  built-in name and each Greek `.cmd()` call — this is what caught that no
  `autoOperatorNames` change was actually needed (see the entries above).
  Live in the running app: `Ctrl+B` inserts `β`; `β:=5` then `β+1=6` on
  the next line; `Ctrl+G`/`Ctrl+Shift+D` in a matrix cell render `[γ  Δ]`;
  `Ctrl+Shift+S` confirmed to NOT insert Σ; `sin(0)=0`, `ln(1)=0`,
  `sech(0)=1` (upright via `\operatorname{}`), `sin(k)` broadcasting over
  a range, and `arcsin(sin(0.5))=0.5`; `@sin(x)` plots an unbroken sine
  curve as a graphing regression check; the pre-existing `$i^2=` sum
  shortcut re-verified unchanged.
- 2026-11-24: Fixed Greek-letter shortcuts (`Ctrl+<letter>`/`Ctrl+Shift+
  <letter>`) not working as the literal first keystroke on a brand-new
  pending line (see "Greek letters" entry above for the fix itself).
  `bin/build`/`bin/test` both clean (7/7). Live in the running app,
  driven with `wtype` and screenshotted with `grim`: `Ctrl+B`/`Ctrl+Shift+
  D` as the first keystroke on a fresh line now insert β/Δ and convert
  the line to math; `\`/`;`/`$`/`@`/heading-marker as first keystroke,
  and `Ctrl+<letter>` mid-line in an already-converted field, all
  re-verified unchanged.
- 2026-11-24: Matrix functions & equation solver (see the entry above
  for the full design). `bin/build`/`bin/test` clean (7/7). Live-tested
  twice over: once in the compiled app for basic matrix-naming, and a
  full pass in a plain-Chrome-hosted copy of `notebook.html` (this is
  pure static HTML/JS with no build step — serving it directly gives
  real DOM/console inspection the compiled Qt app doesn't) after an
  initial Qt-app test of a named-definition transpose appeared to fail
  silently; re-running the identical keystroke sequence in Chrome
  produced the correct result every time, pointing to a Qt-app-specific
  test artifact (most likely the known terminal-focus-steals-keystrokes
  hazard already in `.claude/agents/frontend.md`'s lessons) rather than
  a real bug — see that file's newest entries for the full writeup and
  for two new live-testing gotchas hit along the way (MathQuill
  exponent blocks don't auto-exit after typing, and Ctrl+A can select/
  delete across the *entire* document, not just the focused field).
- 2026-11-24: Statistical functions (see the entry above for the full
  design). `bin/build`/`bin/test` clean (7/7). Verified independently
  (not just trusting the delegated report) in a plain-Chrome-hosted
  copy of `notebook.html`: `i:=0…4` then `mean(i)=2`, `median(i)=2`,
  `variance(i)=2`, `stdev(i)=1.414214` — all correct. A
  `.claude/agent-memory/` directory the implementing session created
  was removed before committing, on the mistaken belief it was an
  invented location — **corrected 2026-11-25: it wasn't.**
  `.claude/agents/frontend.md`'s `memory: project` frontmatter makes
  `.claude/agent-memory/frontend/` that agent's real, documented,
  meant-to-be-committed persistent memory (see the manager's own
  `verification-discipline` memory for the full correction). That
  directory was deleted this same mistaken way at least three times
  across sessions before being caught and fixed on 2026-11-25 — this
  entry's deleted content (the jQuery-load-order gotcha) is
  unrecoverable beyond what was already duplicated into
  `.claude/agents/frontend.md`'s lessons-learned section, but the
  directory itself is restored and preserved going forward.
- 2026-11-24: Paper mode made the default (see the entry above). One-
  line change (`<html class="paper">`). `bin/build`/`bin/test` clean
  (7/7). Verified live in the compiled app: fresh launch shows white
  background; typed text renders in black ink, confirming it's not
  white-on-white.
- 2026-11-24: Inline code-completion for built-in function names (see
  the entry above for the full design). `bin/build`/`bin/test` clean
  (7/7). Verified independently in a visible Chromium window via
  `claude-in-chrome` (not just trusting the delegated report): typing
  "de" in a math line showed "det" positioned at the cursor, Enter
  inserted `det( )` correctly rendered upright; typing exactly "sin"
  then Enter inserted `sin( )`, not `sinh(...)`; Escape after typing
  "me" left the field untouched and still usable (continuing to type
  "an(5)" produced the expected literal `mean(5)`, confirming nothing
  was inserted); "sin(0)=" typed entirely by hand evaluated to `0`
  unchanged; `Ctrl+B` still inserted β as a fresh line's first
  keystroke; typing "tr" inside a matrix cell showed "trace" positioned
  correctly there too.
- 2026-11-25: `e` and `\pi` seeded as built-in numeric constants
  (`Math.E`/`Math.PI`) in every fresh `vars` object the evaluator
  creates (`src/notebook.html`): the main per-line `collectDefinitions()`
  scan and both the 2D and 3D graph-sampling loops, via a shared
  `freshBuiltinVars()` helper so there's one seed, not three copies.
  Seeded *before* each scope's own `:=` scan runs, so a user's own
  `e:=5` overwrites the default in the same object with no special-
  casing in lookup — confirmed live: `e:=5` on an earlier line makes a
  later `e=` read `5`, not `2.718282`. Deliberately scoped to `e`/`π`
  only; `i`/`j` (complex numbers) are untouched — a separate, much
  bigger item if ever requested. Verified live (headless Chromium over
  the DevTools Protocol directly, `claude-in-chrome` wasn't connected
  in this environment — see `.claude/agents/frontend.md`'s dated entry
  for the fallback recipe, now documented for next time): `e=` →
  `2.718282`; `\pi=` (via a real `Ctrl+P` keydown, not just inserting
  the token directly) → `3.141593`; `e:=5` override as above; graph
  lines `@e^x` (2D) and `e^x - y\cdot\pi` (3D) both sample and render
  correctly; `f(e):=e+1` then `f(10)=` → `11`, confirming a function
  parameter named `e` shadows the constant inside that function.
- 2026-11-25: Scalar-matrix multiplication (`2*A`/`A*2`, either
  operand order, including implicit-multiplication juxtaposition like
  `2A`). `broadcast()` (`src/notebook.html`) gained a 4th `opName`
  parameter, passed as `"*"` only from `parseTerm()`'s two actual
  multiplication branches; when one operand is a matrix
  (`isMatrixValue()`) and the other a plain number, it now returns a
  new matrix (`makeMatrix()`) with the scalar multiplied into every
  cell, instead of throwing. Every other matrix-operand combination
  (`+`, `-`, matrix×matrix, `/` either direction) still throws the
  same pre-existing "matrix arithmetic ... is not supported" error,
  confirmed by re-testing each one, not just assuming the scoped fix
  didn't touch them. Display needed no new code — `evaluateAndAppend()`
  already routes any matrix result through the existing
  `appendMatrixResult()` widget. `bin/build`/`bin/test` clean (7/7).
  Verified live (headless Chromium over the DevTools Protocol,
  `claude-in-chrome` wasn't connected in this environment — see
  `.claude/agents/frontend.md`'s 2026-11-25 fallback recipe): built a
  real named matrix `A` through the app's own matrix-entry keyboard
  path, then confirmed `2*A=` and `A*2=` both produce `[[2,4],[6,8]]`
  from `A=[[1,2],[3,4]]`; `A+A=`, `A*A=` (matrix×matrix), `A/2=`, and
  `2/A=` all still produce no matrix result (still throw), unchanged.
- 2026-11-26: Backspace-deletes-pristine-structure, the program-block
  nesting cap lift, and general matrix indexing (`M_{i,j}`/`M_i`,
  `row()`/`col()`) all shipped and live-verified this session (details
  above). `bin/build`/`bin/test` re-confirmed clean (7/7) afterward —
  expected, since nothing backend-side changed, but run anyway as a
  whole-project sanity check before closing out the session. Also
  brought `README.md`'s matrix/program-block sections up to date,
  including documenting `det`/`trace`/`^T`/`^-1`/`solve`/`mean`/
  `median`/`variance`/`stdev`, which had been shipped and verified in
  this file across earlier sessions but never made it into the
  user-facing feature list.
- 2026-11-26: **In-app F1 help overlay**, the first in-app help
  livesheet has ever had — user-requested, with the trigger keys
  (`F1` open/toggle, `Esc` close) specified by the user up front. A
  hand-written static cheat sheet (no generation mechanism, matching
  this project's no-build-step philosophy) covering every section of
  `README.md`'s feature list as compact key→action tables rather than
  prose, built entirely in `src/notebook.html` (no QML/backend
  involvement — confirmed the whole notebook already lives in the
  WebEngineView). Design highlights: `F1` is checked first in the
  global keydown listener, unconditionally, before every other
  branch — so once help is open, its own `Escape` meaning ("close
  help") always wins over the pre-existing function-name completion
  dropdown's own `Escape` handler further down the same listener,
  with no ordering bug to worry about; opening help also proactively
  dismisses that dropdown if it's showing, so the two overlays never
  stack. Focus/cursor state is captured before opening and restored
  exactly on close — a MathQuill-backed field (math/graph/matrix-
  cell/program) just needs `.focus()` again (MathQuill tracks cursor
  position internally, independent of DOM blur); a plain
  pending/heading contenteditable line needed its own saved-character-
  offset-plus-`TreeWalker` restore, since the DOM has no built-in
  "focus at character offset N." Colors read `var(--bg)`/`var(--ink)`
  exactly like every other themed element, confirmed to flip correctly
  with `Ctrl+L` in both directions. One content inaccuracy introduced
  by the implementing session's own paraphrase of README's Greek-letter
  footnote was caught and fixed at review time (it implied
  `Ctrl+Shift+S`/`Ctrl+Shift+P` were "app shortcuts" — they're actually
  just deliberately left unmapped, to keep `Σ`/`Π` from silently
  doubling as plain variable names). Live-verified (claude-in-chrome):
  F1 opens from every focus state (empty pending line, math field with
  real cursor position, matrix cell, program field) and Escape restores
  the exact same field/cursor each time with zero document-state
  change; F1 toggles both directions; opening help while the
  completion dropdown is showing cleanly dismisses it; Ctrl+L while
  help is open updates its colors live; Ctrl+S/Ctrl+Q/Greek-letter/
  program-block shortcuts all unaffected. `bin/build`/`bin/test`
  reconfirmed clean (7/7) afterward. Also checked, at review time, that
  nothing native competes for F1: grepped `src/*.qml`/`src/*.cpp`/
  `src/*.h` for any existing `F1`/`Qt.Key_F1`/`StandardKey.Help`
  binding (none exists), and confirmed F1 isn't one of Chromium's own
  hard-wired keyboard accelerators the way `Ctrl+Shift+R`
  (hard-reload, this project's own past incident) or `Ctrl+Shift+I`
  (devtools) are — unlike those two, "Help" in real Chrome is a menu
  action, not a global accelerator, so there's nothing underneath for
  the page's own unconditional `preventDefault()` to need to race
  against. (Attempted to also confirm this by launching the real
  compiled binary and sending a literal F1 keypress under Hyprland;
  blocked by this sandbox's non-standard Lua-based `hyprctl dispatch`
  syntax rejecting both classic and guessed Lua-call forms for
  workspace-switching/window-focusing, so that specific visual
  confirmation wasn't completed — the grep + Chromium-accelerator
  reasoning above was judged sufficient in its place.)
