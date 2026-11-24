# TODO

Running backlog of requested features, tracked by the manager session
so a new session can pick up immediately without re-reading the whole
conversation history. Update status here as items move; once something
is actually done and verified, its details belong in `PROGRESS.md`, not
here — this file should only ever describe what's *not* done yet, plus
enough design context that whoever picks an item up next doesn't have
to re-derive decisions already made.

Last updated: 2026-11-24 (matrix functions & equation solver shipped —
see `PROGRESS.md`). Everything below this line is still open.

## Queued — statistical functions (small, quick follow-on)

`mean`/`median`/`variance`/`stdev`/... — numeric, likely scoped over a
range or matrix value. Deliberately left undone when matrix functions/
equation solver shipped (see `PROGRESS.md`): matrices are now real
values with the same `vars`/`funcs` plumbing a stats builtin would use,
so this should be a small, mechanical addition, not a redesign. Exact
function set still to be decided — start from the list above and check
with the user if anything beyond it seems warranted.

## Queued — symbolic calculus (derivatives & integrals; big, needs its own design pass)

Originally bundled into "Batch 2" as plain numeric derivatives/
integrals (finite differences, quadrature — the same per-point
sampling style graphing already uses). Re-scoped 2026-11-24: the user
was asked explicitly whether an indefinite integral (which has no
numeric equivalent — it's an antiderivative *formula*, not a value at
a point) should be dropped, limited to polynomials, or built as a real
symbolic antiderivative, and chose **full symbolic antiderivative**.
That decision pulls derivatives along with it for consistency (a formula
result for one but not the other would be a strange asymmetry), and
means this item is genuine computer-algebra work, not an extension of
the existing numeric evaluator.

**Why this is its own item, not a sub-task of anything else**: every
line of `evaluateLatex()` today evaluates LaTeX directly to a number —
there is no expression-tree/AST representation anywhere in the
codebase. Symbolic differentiation and integration both require
building and manipulating an actual tree (apply a rule, get back another
expression, not a number), then rendering that tree back to LaTeX for
display. That's a new representation layered under the existing
evaluator, not a new builtin function alongside `sin`/`det`/etc. No
design work has been done yet on what that tree's node shape looks like
or how it coexists with the direct-evaluation path (replace it, or run
alongside it for symbolic-only call sites?) — that's the next
delegation's first job on this item.

**Agreed v1 scope** (from the user's chosen option, not to be expanded
without checking back): the standard forms — polynomials (power rule),
trig, exp/log — and the chain rule for simple compositions. Not a
general-purpose CAS; keep it bounded to what a derivative/integral
button on a calculator-grade tool needs, not what a full computer-
algebra system would eventually grow into.

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

## Queued — F1 help overlay

Research done (Omarchy global bindings, grepped every default Hyprland
binding file — zero matches for a literal F-key anywhere; media/
brightness keys use separate `XF86...` hardware codes, no overlap at
all — so all twelve are free with zero compositor-level collision
risk; and general F-key UX convention, web-searched): no universal
convention beyond **F1 = Help**, which fits this app's keyboard-centric,
no-menu-bar design well — the same spirit as the F-key command bar
classic DOS-era keyboard-driven apps (WordPerfect, Lotus 1-2-3) used,
which is squarely the kind of app livesheet already resembles.

**Decided design, deliberately small rather than forcing all 12 keys to
do something**: `F1` toggles a full "Shortcuts & Help" overlay listing
every current shortcut grouped by category (line-type starters, math-
field operators, range & Σ/Π, Greek letters, built-in functions,
graphing, app-level) with a one-line description each — essentially
README's own shortcut tables, surfaced inside the app instead of making
the user tab out to read a file. `Escape` closes it (confirmed unused
in `src/notebook.html`, and the near-universal "dismiss an overlay"
key). **F2–F12 deliberately left unassigned** — assign one only when a
real, concrete need shows up, not preemptively. Keep the overlay's
content and README's shortcut tables from drifting apart — decide
whether that means generating one from the other, or just a comment on
both sides pointing at each other as a manual-sync reminder; silent
drift between them is the one outcome to avoid.
