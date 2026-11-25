# TODO

Running backlog of requested features, tracked by the manager session
so a new session can pick up immediately without re-reading the whole
conversation history. Update status here as items move; once something
is actually done and verified, its details belong in `PROGRESS.md`, not
here — this file should only ever describe what's *not* done yet, plus
enough design context that whoever picks an item up next doesn't have
to re-derive decisions already made.

Last updated: 2026-11-25 (`e`/`π` built-in constants, scalar-matrix
multiplication, and both phases of symbolic calculus — derivatives and
indefinite integrals — shipped; see `PROGRESS.md`). Everything below
this line is still open.

## Queued — program blocks (biggest item, sequenced after the above)

A real programming construct inside livesheet, styled as close to math
notation as possible — explicitly requested: "for each element in
collection" should use the *official* math symbol for that. Sequenced
last on purpose: it's large enough to deserve undivided attention, and
can lean on whatever array/collection infrastructure Batch 2's matrix
work ends up building.

**Confirmed against Mathcad's own real "Programming" feature** (PTC's
own docs/tutorials, web-searched, not guessed):
- `←` for **local assignment** inside a program — distinct from `:=`,
  the worksheet-level definition operator. This is the actual reason
  Mathcad has two different assignment symbols at all.
- `∈` for a **for-each** loop's binding (`for i ∈ 0…10`, or over any
  other collection/array value) — what the user asked for by name, and
  also literally what Mathcad itself uses.
- `if` / `otherwise` for conditionals, `while` for a condition-driven
  loop, `break` / `continue` / `return` for control inside a loop or
  program — Mathcad renders these as literal keyword operators (not a
  special glyph). Whether MathQuill's `autoOperatorNames` mechanism
  (see the trig/log work already shipped, `PROGRESS.md`) already covers
  these or needs extending is unconfirmed — check before assuming
  either way, the same empirical-check standard that work was held to.
- A program block evaluates to its **last statement's value** (Mathcad
  convention) unless an earlier `return` exits it first.

**The hard architectural problem, not yet solved — this is the real
scope of this item**: every line type so far (math, matrix, graph) is
*flat* — a matrix is a 2D grid of cells, but a program's body is a
*tree* (a `for`/`while`/`if`'s body is itself a sequence of statements,
which can itself contain another `for`/`if`, to arbitrary depth in real
Mathcad). Building a genuinely recursive nested-block editor (indent-
aware navigation, a bracket that visually grows/shrinks per nesting
level, Tab/arrow movement crossing nesting boundaries) in the same
keyboard-driven single-field-per-line model this app has used
throughout is the single biggest design lift of the whole project —
bigger than matrices and graphing combined.

**Decided scope for a first version**, to keep this achievable rather
than attempting Mathcad's full generality in one pass: one level of
nesting only — a program block can contain a sequence of local
assignments (`x ← expr`), *one* `for`/`while` loop, *one* `if`/
`otherwise`, and a `return`/trailing expression, but a loop's or
conditional's own body is restricted to plain statements (no loop
inside a loop, no if inside a for, in v1). Revisit once this is built
and in use — extending nesting depth later should be additive, not a
rewrite, if a statement's body is designed from the start as "a list of
statements that isn't yet allowed to recurse," not hard-coded as "a
list of only assignment lines."

**Block-start shortcut**: no single well-documented bare-key Mathcad
convention exists for inserting a program block (unlike `;`/`$`/`#`/`@`,
which all had one) — Mathcad inserts it via a toolbar/ribbon. `{` is
the proposed shortcut (unclaimed; visually matches how a program block
is bracketed and how piecewise-function notation already uses `{`) —
open to override before it's built.

