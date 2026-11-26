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
- The same shortcut also defines a single-letter **function** of one or
  more single-letter parameters, e.g. `f(x):x^2+4` renders as
  `f(x) := x^2 + 4`, or `f(x,y):\sqrt{x^2+y^2}` renders as
  `f(x,y) := \sqrt{x^2+y^2}`. Call it later with a number or expression
  per parameter, e.g. `f(4)` followed by `=` gives `= 20`, or
  `f(2,3)` followed by `=` gives `= 3.605551`
- `;` (Mathcad's own shortcut for its range operator) defines a **range
  variable** when used in place of `:=`'s value, e.g. `i:0;5` renders as
  `i := 0…5` — the integers 0 through 5. `i:0,2;10` renders as
  `i := 0,2…10` for a step of 2 (0, 2, 4, ..., 10) instead of 1. Any
  formula that uses a range variable evaluates once per value in the
  range, the same as Mathcad: with `i := 0…5` defined, `i^2` followed by
  `=` gives `= 0, 1, 4, 9, 16, 25` rather than a single number
- `$` and `#` (Mathcad's own bare shortcuts for its summation/product
  operators) insert **Σ**/**Π**, which sum/multiply the expression typed
  right after them over whatever range variable it contains, e.g. with
  `i := 0…5` defined, `$i^2` followed by `=` gives `= 55`
  (0+1+4+9+16+25) and `#i` (with `i := 1…4`) followed by `=` gives
  `= 24` (1·2·3·4). Σ/Π bind tightly, like unary minus, to just the next
  term — summing a multi-term expression needs explicit parens, e.g.
  `$(i+1)=`
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
- `e` and `π` (type `π` via `Ctrl+P`, see Greek letters below) are
  built-in constants — no definition needed, e.g. `e=` gives
  `= 2.718282` and `\pi=` gives `= 3.141593`. Still ordinary identifiers
  otherwise: defining your own `e:5` on an earlier line shadows the
  constant for every line after it, the same as any other definition
- A scalar times a named matrix (`2*A` or `A*2`, either order — see
  matrices below) multiplies every cell elementwise; `+`/`-`/matrix
  times matrix/division between two matrices is still unsupported
- Common named functions evaluate directly, no definition needed:
  `sin cos tan sec csc cot`, their inverses (`arcsin arccos arctan arcsec
  arccsc arccot`), the hyperbolic versions (`sinh cosh tanh sech csch
  coth`) and their inverses (`arcsinh arccosh arctanh arcsech arccsch
  arccoth`), plus `ln` (natural log), `log` (base 10), and `exp`. Just
  type the name — MathQuill renders it upright automatically, the same
  as `sin` always has — e.g. `sin(0)` followed by `=` gives `= 0`, and
  `sin(i)` with a range variable `i` defined broadcasts the same way any
  other operator does, e.g. `= 0, 0.841471, 0.909297, 0.14112` for
  `i := 0…3`
- `Ctrl+<letter>` / `Ctrl+Shift+<letter>` insert a lowercase/uppercase
  Greek letter (Mathcad's own convention — see the Shortcuts table
  below for the full mapping), usable as an ordinary variable name,
  function name, or function parameter just like a Latin letter, e.g.
  `Ctrl+B` then `:5` defines `β := 5`. **Known limitation**: only works
  once the line is already a math field — unlike every other shortcut
  here, it does nothing as literally the first keystroke on a
  brand-new line (type any other character first, then it works). Fix
  queued, see `TODO.md`
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

`A:[` on a math line showing exactly `A:=` (nothing else typed yet)
names the matrix that follows, the same way `A:3` becomes `A := 3` —
a **named matrix** is a real value later lines can use:

- `det(A)` (determinant), `trace(A)` (sum of the diagonal), `A^T`
  (transpose), and `A^-1` (inverse, Gauss-Jordan elimination) — the
  last two are typed as an ordinary exponent, Mathcad's own
  convention, not a function call. All four require a square matrix
  except transpose
- `solve(expr, x)` solves a single equation for `x` (Newton's method);
  `solve(A, b)` solves the linear system `Ax = b` for a named matrix
  `A` and column vector `b` — which call you get depends on whether
  the second argument is already a matrix-bound variable
- `mean(A)`, `median(A)`, `variance(A)` (population, divide-by-`n`),
  and `stdev(A)` — same four statistics a range variable's own values
  accept, reading the matrix flattened row-major
- `A_{i,j}` (typed as a subscript) is the scalar element at row `i`,
  column `j`, 1-indexed; `A_i` is the `i`-th element when `A` is a
  single row or column (an error otherwise, telling you to use
  `A_{i,j}` instead)
- `row(A, i)` / `col(A, j)` return a new matrix — just that row or
  column of `A`, 1-indexed — e.g. to sum a matrix's second row via a
  program block's `for` loop: `for x \in row(A,2)`
- A result that's itself a matrix (`A^T`, `solve(A,b)`, `row(A,i)`, ...)
  displays as a real typeset matrix next to the `=`, not a flat list

None of this applies to an unnamed matrix (the original bare `[` on a
pending line, with no `name:=` in front) — that's still just a typeset
widget, not a value.

`@` at the start of a line (Mathcad's own bare shortcut for inserting an
X-Y plot) starts a **graph** instead of a math field:

- Type the expression to plot in terms of `x` right after the `y =`, e.g.
  `x^2`, or call a function defined on an earlier line, e.g. `f(x)`
- The plot updates live as you type, over a fixed `x ∈ [-10, 10]` domain
  with the y-axis auto-scaled to fit — there's no separate placeholder for
  the axis limits, unlike a real Mathcad X-Y plot
- A part of the curve that can't be evaluated (dividing by zero, an
  undefined reference, ...) leaves a gap instead of a spurious straight
  line across it, the same way a real graph doesn't connect across an
  asymptote
- The same field also plots a **3D surface** `z = f(x, y)` instead,
  automatically — if the expression actually uses `y` as a second free
  variable (rather than leaving it unused, or using an earlier `y := ...`
  definition as an ordinary value), the label switches live from `y =`
  to `z =` and the plot becomes a wireframe mesh. Try `x^2+y^2` for a
  paraboloid. Sampled on a much coarser 15×15 grid than the 2D curve's
  201 points (a dense wireframe just looks like noise, and it's an
  N² cost instead of 2D's N), projected with a fixed isometric
  projection and auto-scaled/centered to the computed z-range. A grid
  point that can't be evaluated leaves a gap in the mesh lines through
  it, same principle as the 2D asymptote case, just per mesh segment
- **Enter** finishes the graph and starts a new line below it;
  **Backspace** on an empty expression removes the graph, same as an
  empty math line
- Exported as just the defining equation (`y = ...` or, for a 3D
  surface, `z = ...`, as a normal display equation) rather than a
  plotted image — turning sampled points into an actual embedded plot in
  the `.tex`/`.pdf` output would need a real plotting package (e.g.
  `pgfplots`), which is out of scope here

Typing `d`, `/`, `d`, a variable name, then the expression to
differentiate (in Leibniz notation, e.g. `d/dx(x^2)` — the `/` opens an
ordinary fraction, you're just filling in `d` over `d<var>`) followed
by `=` gives the **symbolic derivative** as a formula, not a number
at a point, e.g. `d/dx(x^2)` followed by `=` shows `2x`, and
`d/dx\sin(x^2)` shows `\cos(x^2)\cdot2x`. Covers polynomials (power
rule), `sin`/`cos`/`tan`, `exp`/`ln`/`log`, and the chain rule for
compositions of these — not a general computer-algebra system.

`Ctrl+Shift+I` inserts an **indefinite integral** template (`\int(...)dx`
— fill in the integrand between the parens, with the differential
variable already in place after); followed by `=` gives the symbolic
antiderivative, e.g. filling in `1/x` gives `\ln(x)`, and `\sin(3x)`
gives `-\cos(3x)/3`. Covers the same standard forms as the derivative
above, plus linear substitution (`∫f(ax+b)dx`) for simple
compositions — general substitution, `\tan`/`\ln`/`\log` integrals,
and anything needing integration by parts are out of scope and leave
the line unevaluated rather than guessing.

`{` at the start of a line (or right after `f(x):=`, making it that
function's body) starts a **program block** — a real sequence of
statements, for short algorithms that don't fit in a single
expression:

- Inside a statement, `:` means **local assignment** (`x\leftarrow
  expr`, Mathcad's own program-scoped assignment symbol — distinct
  from the worksheet-level `:=` everywhere else) rather than defining a
  sheet-wide variable
- `Ctrl+Shift+R` inserts a **for** loop (iterating a range variable, a
  named matrix flattened, or one row/column of a named matrix via
  `row()`/`col()` — see matrices above), `Ctrl+Shift+W` a **while**
  loop, `Ctrl+Shift+N` an **if/otherwise** conditional, and
  `Ctrl+Shift+B`/`T`/`E` insert **break**/**continue**/**return** — any
  number of loops/`if`s, and any of them can nest inside another's
  body, to any depth
- A program evaluates to its **last statement's value**, unless an
  earlier `return` exits it first — e.g. a bare program block with
  `s\leftarrow0`, a `for i` loop adding `i` into `s`, then a trailing
  `s`, followed by `=`, sums the loop
- Comparisons (`<`, `>`, `=`, `\leq`, `\geq`, `\neq`) work inside a
  `for`/`while`/`if`'s own condition field — they're not available on
  an ordinary math line otherwise
- **Tab**/arrow keys navigate between a program's fields; **Enter**
  adds another statement in the current sequence; **Backspace** on an
  empty statement removes it (emptying a bare program's only statement
  reverts the line back to pending, same as a 1×1 matrix). Backspace on
  an empty `for`/`while`/`if` also removes the whole loop/`if`, but
  only while it's still untouched since you inserted it — once you've
  typed anything into it anywhere, Backspace just moves between fields
  instead, so real work is never silently discarded
- As a function's body (`f(x):={...`), call it afterward exactly like
  any other function, e.g. `f(5)` followed by `=`

## Shortcuts

| Key      | Action   |
|----------|----------|
| `Ctrl+S` | Export   |
| `Ctrl+Q` | Quit     |
| `Ctrl+L` | Toggle a dark sheet look (vs. the default white-paper/black-ink) — session-only, not remembered between launches |
| `Ctrl+Shift+I` | Insert an indefinite-integral template |
| `Ctrl+Shift+R` | Inside a program block: insert a `for` loop |
| `Ctrl+Shift+W` | Inside a program block: insert a `while` loop |
| `Ctrl+Shift+N` | Inside a program block: insert an `if`/`otherwise` |
| `Ctrl+Shift+B` | Inside a program block: insert `break` |
| `Ctrl+Shift+T` | Inside a program block: insert `continue` |
| `Ctrl+Shift+E` | Inside a program block: insert `return` |

### Greek letters

`Ctrl+<letter>` for lowercase, `Ctrl+Shift+<letter>` for uppercase, in a
math field, graph expression, or matrix cell. `a c v x` stay copy/paste/
cut/select-all and plain `s q l` stay the app shortcuts above, so none of
those six get a Greek mapping; `Ctrl+Shift+S`/`Ctrl+Shift+P` are also left
unmapped even though plain `s`/`q`/`l` are reserved only in their
unshifted form — **Σ**/**Π** already mean something specific to this app
(the summation/product shortcuts above), so letting them double as plain
variable names too would make `Σ := 5` followed by `Σ+1` silently
misevaluate instead of failing loudly.

| Lowercase | Letter | | Lowercase | Letter | | Lowercase | Letter |
|---|---|---|---|---|---|---|---|
| `Ctrl+B` | β | | `Ctrl+J` | ξ | | `Ctrl+R` | ρ |
| `Ctrl+D` | δ | | `Ctrl+K` | κ | | `Ctrl+T` | τ |
| `Ctrl+E` | ε | | `Ctrl+M` | μ | | `Ctrl+U` | υ |
| `Ctrl+F` | φ | | `Ctrl+N` | ν | | `Ctrl+W` | χ |
| `Ctrl+G` | γ | | `Ctrl+O` | ω | | `Ctrl+Y` | ψ |
| `Ctrl+H` | η | | `Ctrl+P` | π | | `Ctrl+Z` | ζ |
| `Ctrl+I` | ι | | | | | | |

| Uppercase | Letter | | Uppercase | Letter | | Uppercase | Letter |
|---|---|---|---|---|---|---|---|
| `Ctrl+Shift+G` | Γ | | `Ctrl+Shift+J` | Ξ | | `Ctrl+Shift+F` | Φ |
| `Ctrl+Shift+D` | Δ | | `Ctrl+Shift+U` | Υ | | `Ctrl+Shift+Y` | Ψ |
| `Ctrl+Shift+Q` | Θ | | `Ctrl+Shift+L` | Λ | | `Ctrl+Shift+O` | Ω |

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
