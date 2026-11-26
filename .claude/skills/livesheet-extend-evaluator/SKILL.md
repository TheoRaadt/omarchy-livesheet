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
- **A typed `_` (subscript) is NOT wrapped in braces for exactly one
  plain character** (`M_2`, `M_i`), the same "single character skips
  braces" rule `^` already has (see above) — confirmed as its own,
  separate empirical fact for `_` rather than assumed by analogy to
  `^`, since this build's two superficially-similar template
  mechanisms aren't guaranteed to share every quirk. Anything longer
  (a multi-digit number, a comma-separated pair, a Greek command, a
  negative number) IS braced (`M_{23}`, `M_{2,3}`, `M_{\beta}`,
  `M_{-1}`). An empty subscript renders as `M_{ }` (a literal space
  inside braces), never a bare dangling `_` — so a parser branching on
  "bare single char vs. braced group" has no third, brace-less-and-
  empty case to worry about.
- **A typed `/` key always produces `\frac{}{}` in this MathQuill
  build — never `\div`.** `\div` is a separate, real LaTeX token this
  evaluator's grammar can still parse, but nothing in this app's own
  keybindings ever types it; it's effectively dead code as far as live
  user input goes. If you're adding behavior to "the division
  operator" and you only touch the `\div` call site because that's the
  one literally named "divide," you've silently missed the actual path
  real typing takes — trace how a user types an operator (not just
  what the grammar accepts) before deciding a change to it is
  complete.

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

## A shared helper's existing behavior may be right for only SOME of its callers

`broadcastUnary()` is called by both unary minus and (indirectly) every
`sin`/`cos`/`ln`-style built-in — and separately, when it gained a new
"pass a tagged value's metadata through unchanged" behavior for one new
caller (physical units' quantity type, letting `-5kg` correctly stay
`-5kg`), that same behavior was silently WRONG for a different existing
caller that happened to route through the same function too: un-indexed
`\sqrt` also used `broadcastUnary()` at the time, and a square root
needs to actually transform that metadata (halve a quantity's
dimensions), not pass it through untouched the way negation correctly
does. Before changing (or extending) a shared helper's behavior for the
caller you're actively working on, enumerate every OTHER caller of that
same function and check whether its correctness requirement actually
matches the one you're adding — don't assume "this is the only place
that calls it" or "every caller wants the same thing."

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
