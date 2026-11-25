---
name: livesheet-extend-evaluator
description: Guidance and concrete MathQuill-LaTeX-emission gotchas for extending livesheet's evaluateLatex() grammar/parser in src/notebook.html — adding a new operator, built-in function, or parsing rule. Covers what MathQuill actually emits for typed input (not the idealized LaTeX spec), the decimal-merge trap, how to safely add a new top-level grammar entry point with zero regression risk by proving a character never appears in existing input, the "one statement/row kind can double as two different grammatical things" pattern, and the autoOperatorNames empirical-check/replace-not-extend gotcha. Use this skill whenever adding a new operator, comparison, built-in function, or parsing rule to evaluateLatex() or any similarly-shaped LaTeX-parsing function in notebook.html.
---

# Extending livesheet's evaluator (src/notebook.html)

`evaluateLatex()` is a hand-rolled recursive-descent parser over the
LaTeX **MathQuill actually produces** for typed input — not idealized
LaTeX, not what the LaTeX spec says is equivalent. The gap between
those two things is where most of this codebase's real parsing bugs
have come from.

## What MathQuill actually emits (confirmed, not assumed from the spec)

- **A typed `(`/`)` emits `\left(`/`\right)` in `.latex()`, never
  bare `(`/`)`.** A parser branch that only recognizes bare parens
  silently never matches real typed input. Handle both forms wherever
  parens matter.
- **A single-character exponent is NOT wrapped in braces** (`a^T`,
  not `a^{T}`) even though a multi-character one is (`a^{12}`). Any
  code branching on an exponent's content needs to handle both the
  braced-group and the bare-single-character case — this is a
  property of what this specific MathQuill build emits, not something
  you'd predict from the LaTeX spec (which treats braces as always
  optional-but-conventional).
- **The implicit-multiplication lookahead** (deciding whether `2x` or
  `x\sqrt{...}` means "multiply") **must not treat every backslash as
  "yes, a new atom is starting."** A `\right)` (or `\end{...}`, etc.)
  closing an *enclosing* group is also a backslash command, but it
  means the opposite — the current group is ending, not a new factor
  starting. Enumerate the specific backslash commands that actually
  start a new atom (`\sqrt`, `\frac`, `\left(`, a recognized function
  name, ...) instead of matching any leading `\`.
- **Two literal `.` characters typed right after a digit get silently
  merged by MathQuill into one decimal point** (sensible for real
  numbers, but it means a naively-chosen two-dot marker like `0..10`
  for a range silently becomes `0.10`). Never use two adjacent literal
  characters as a multi-character marker for a new piece of syntax —
  use a single atomic MathQuill command instead (inserted via
  `mathField.cmd()`, e.g. `\ldots`), which can't be split or merged by
  MathQuill's own typing rules the way repeated plain characters can.
- **A shorter `a`-prefixed inverse-function name (`asin`, `acosh`,
  ...) is never auto-recognized as one token** — only `arc`-/`ar`-
  prefixed forms are in MathQuill's generated name list (see the
  `autoOperatorNames` note below). Typing `asin(` live actually
  produces a separate italic variable `a` immediately followed by a
  recognized `\sin`, rendering as `a\sin\left(...\right)` — i.e. "a
  times sin(...)", a different, already-meaningful expression. Don't
  special-case the evaluator to reinterpret that token sequence as the
  inverse function — it would silently break the legitimate "`a` is a
  real variable" case for anyone who actually meant that.
- **`autoOperatorNames` has a much larger effective default table than
  the literal strings visible in the minified source suggest** — its
  startup code expands a short hardcoded list programmatically at load
  time into every `arc`-/`h`-/`ar`- combination. Verify what's
  actually recognized empirically (load the real bundled
  `mathquill.min.js`, `.typedText()` the name, read back `.latex()`),
  not by reading the minified text. And `MQ.config({autoOperatorNames:
  "..."})`'s setter **replaces** the whole table with only the names
  you give it — it does not extend the default list, so passing a
  short string of just the names you think are missing would silently
  drop every other default name too.

## Adding a new top-level grammar entry point (e.g. a new precedence level) safely

You can sometimes make a brand-new parse function the new top-level
entry point of `evaluateLatex()` (replacing `var result =
<old-entry>()` with `var result = <new-entry>()`) with **zero
regression risk** to every existing line's own evaluation — but only
when you can actually prove the new syntax's trigger character(s)
never appear in the LaTeX any existing line type is ever evaluated
with. This project added comparison operators (`<`, `>`, `=`, `\leq`,
`\geq`, `\neq`) this way: it's safe specifically because
`insertChar()`'s own `"="` branch always captures a math line's
expression text *before* that keystroke is inserted into the field at
all, so no pre-existing line's evaluated LaTeX can ever contain a
literal `=`/`<`/`>` character already. Before using this pattern for
something new, verify the equivalent fact for your new trigger
character(s) — don't assume it by analogy.

## "One statement/row kind can double as two different grammatical things"

If your feature has a "last item is special" convention (e.g. "a
block evaluates to its last statement's value"), don't assume every
item of that kind is shaped the same way as the ones you built and
tested first. This project's program blocks hit this concretely: a
statement row's `"assign"` kind can mean either `name \leftarrow expr`
(bind a name) or a bare expression (compute, don't bind) — which
matters because the one row that's *required* to exist (the trailing
value) is never forced to be a named assignment, and it's created
using the exact same default statement object every other row uses.
Unconditionally assuming the "primary" shape (here: requiring
`\leftarrow`) threw on the most basic possible example, and wasn't
caught by logic tests that happened to only ever exercise cases with a
redundant trailing self-assignment. If a construct has a privileged
"last/special" position, explicitly test the case where that position
holds the *other*, less-common shape.

## Testing a new grammar rule against definition-scanning functions

If you're testing whether a line correctly registers itself as
something lookup-able (e.g. a new kind of function definition), check
whether the scanning function you're testing with only looks at lines
*strictly before* the one you're testing — `collectDefinitions(line)`
in this codebase does exactly that (`stop = indexOf(line)`), which is
correct for its real use ("what can this line see") but means calling
it on the defining line itself to check "did this register correctly"
is a no-op by construction, not a real test. Use a different, later
line instead — the same way an actual user would call something they
just defined.
