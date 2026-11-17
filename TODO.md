# TODO

Running backlog of requested features, tracked by the manager session.
Update status here as items move; move finished items to `PROGRESS.md`
instead of leaving them here once they're actually done and verified.

## In progress

- **Batch 1 — Greek letters + named built-in functions** (delegated to
  `frontend`, launching now): Greek letters (keybinding), trig +
  inverse trig, hyperbolic + inverse hyperbolic, log/exponential.
  Grouped together because they share one underlying mechanism — see
  "Design notes" below.

## Queued (not started)

- **Batch 2 — calculus & linear algebra**: definite/indefinite
  integrals, single/partial derivatives, standard matrix functions
  (det, inverse, transpose, etc.), statistical functions (mean,
  median, variance, stdev, ... — likely over a range/matrix, exact set
  still to be scoped), and an equation solver (one equation, and
  systems of more than one — the system case almost certainly leans on
  whatever matrix inverse/elimination machinery the matrix-functions
  item above ends up building, so keeping them in the same batch
  matters, not just file-collision avoidance). Queued behind Batch 1 so
  both don't edit `src/notebook.html` at the same time.

## Queued (not started), after Batch 2 — program blocks

The biggest feature requested so far: a real programming construct
living inside livesheet, styled as close to math notation as possible
(explicitly requested: "for each element in collection" should use the
*official* math symbol for that). Sequenced after Batches 1 and 2 on
purpose — not just to avoid file collisions, but because it's large
enough to deserve undivided attention, and it can lean on whatever
array/collection infrastructure Batch 2's matrix work ends up building.

**Confirmed against Mathcad's own real "Programming" feature** (PTC's
own docs/tutorials, web-searched, not guessed):
- `←` for **local assignment** inside a program — distinct from `:=`,
  which stays the worksheet-level definition operator. This is the
  actual reason Mathcad has two different assignment symbols at all.
- `∈` for a **for-each** loop's binding (`for i ∈ 0…10`, or over any
  other collection/array value) — exactly what the user asked for by
  name, and it's also literally what Mathcad itself uses.
- `if` / `otherwise` for conditionals, `while` for a condition-driven
  loop, `break` / `continue` / `return` for control inside a loop or
  program — Mathcad renders these as literal keyword operators (not a
  special glyph), inserted the same way `sin`/`det`/etc. get their
  upright styling (see the trig/log section below on
  `autoOperatorNames` — likely the same mechanism applies here, confirm
  rather than assume).
- A program block evaluates to its **last statement's value** (Mathcad
  convention) unless an earlier `return` exits it first.

**The hard architectural problem, not yet solved — this is the real
scope of this item**: every line type so far (math, matrix, graph) is
a *flat* structure — a matrix is a 2D grid of cells, but a program's
body is a *tree* (a `for`/`while`/`if`'s body is itself a sequence of
statements, which can itself contain another `for`/`if`, to arbitrary
depth in real Mathcad). Building a genuinely recursive nested-block
editor (indent-aware navigation, a bracket that visually grows/shrinks
per nesting level, Tab/arrow movement crossing nesting boundaries) in
the same keyboard-driven single-field-per-line model this app has used
throughout is the single biggest design lift of the whole project —
bigger than matrices and graphing combined.

**Decided scope for a first version**, to keep this achievable rather
than attempting Mathcad's full generality in one pass: one level of
nesting only — a program block can contain a sequence of local
assignments (`x ← expr`), *one* `for`/`while` loop, *one* `if`/
`otherwise`, and a `return`/trailing expression, but a loop's or
conditional's own body is restricted to plain statements (no loop
inside a loop, no if inside a for, etc., in v1). Revisit once this is
built and in use — extending nesting depth later is additive, not a
rewrite, if the block structure is designed with that in mind from the
start (i.e. a statement's body should already be "a list of
statements," just not allowed to recurse yet, not hard-coded as "a
list of only assignment lines").

**Block-start shortcut**: no single well-documented bare-key Mathcad
convention exists for inserting a program block (unlike `;`/`$`/`#`/`@`,
which all had one) — Mathcad inserts it via a toolbar/ribbon. `{` is
the proposed shortcut (unclaimed, and visually matches how a program
block is bracketed, and how piecewise-function notation already uses
`{` for cases) — open to the user overriding this before it's built.

## Queued (not started) — F1 help overlay

Research done (Omarchy global bindings, checked the same way as the
Ctrl/Ctrl+Shift research below; and general F-key UX convention,
web-searched): **F1–F12 are completely unclaimed by Omarchy** — grepped
every default Hyprland binding file, zero matches for a literal F-key
(media/brightness keys use separate `XF86...` hardware codes, not the
F1–F12 codes, so there's no overlap at all). All twelve are free to use
within the app with zero compositor-level collision risk.

On what to actually do with them: the research found no universal
convention beyond **F1 = Help**, which is close to universally
recognized across software going back decades (and fits this app's
whole keyboard-centric, no-menu-bar design better than most — it's the
same spirit as the F-key command bar classic DOS-era keyboard-driven
apps like WordPerfect/Lotus 1-2-3 used, which is squarely the kind of
app livesheet already resembles). Everything else about F-keys
(F2=rename, F5=refresh, ...) is Windows/Office-specific convention that
doesn't map onto anything livesheet has.

**Decided design, deliberately small rather than forcing all 12 keys to
do something**: `F1` toggles a full "Shortcuts & Help" overlay listing
every current shortcut grouped by category (line-type starters: `#`/
`##`/`###`/`[`/`@`/(`{` once program blocks land); math-field operators:
`:` `/` `\` `|` `=`; range & Σ/Π: `;` `$` `#`; Greek letters: the
Ctrl/Ctrl+Shift table; graphing; app-level Ctrl+S/Q/L/L) with a
one-line description each — essentially the README's own shortcut
tables, surfaced inside the app instead of making the user tab out to
read a file. `Escape` closes it (confirmed unused in `src/notebook.html`
today, and is the near-universal "dismiss an overlay" key). **F2–F12
are deliberately left unassigned for now** rather than inventing
speculative uses just because the keys are free — assign one only when
a real, concrete need shows up, not preemptively.

Keep the overlay's content and README.md's shortcut tables from
drifting apart — whoever builds this should decide whether that means
generating one from the other, or just a code comment on both sides
pointing at each other as a manual-sync reminder; either is fine, silent
drift between them is not.

## Design notes (so the next delegation doesn't re-derive these)

- **Greek letters**: `Ctrl+<letter>` for lowercase, `Ctrl+Shift+<letter>`
  for uppercase — confirmed free of Omarchy's compositor bindings
  (every Omarchy `CTRL`-based shortcut also requires `SUPER`). Reserved
  letters: `a/c/v/x` (copy/paste/cut/select-all, must keep working) and
  `s/q/l` in their *plain* form only (livesheet's own export/quit/paper
  shortcuts — `Ctrl+Shift+S/Q/L` are free). Mapping:
  - Lowercase: b→β d→δ e→ε f→φ g→γ h→η i→ι j→ξ k→κ m→μ n→ν o→ω p→π r→ρ
    t→τ u→υ w→χ y→ψ z→ζ (alpha and omicron don't get a slot — see the
    manager's reply in conversation for the full reasoning; omicron is
    visually identical to "o" anyway so that's not a real loss).
  - Uppercase: g→Γ d→Δ q→Θ l→Λ j→Ξ p→Π s→Σ u→Υ f→Φ y→Ψ o→Ω (the 11
    MathQuill actually renders distinctly from their Latin capital).
  - Using Greek letters as real variables/function parameters (not just
    decoration) needs evaluateLatex()'s identifier model extended
    beyond "one Latin letter" to also accept a Greek command token
    (`\alpha`, `\beta`, ...) — touches parseAtom's letter branch and
    both collectDefinitions() regexes (variable and function
    definitions). This is the non-trivial part of this item.
- **Trig/hyperbolic/log/exp functions**: MathQuill already auto-
  recognizes typed letter sequences like "sin", "cos", "ln", "det" as
  upright operator names (`autoOperatorNames`, confirmed in
  `src/mathquill/mathquill.min.js`) — **no new keybinding needed for
  these**, they already look right the moment you type them. MathQuill's
  *default* list covers: `sin cos tan arcsin arccos arctan sinh cosh
  tanh sec csc cot coth` plus `arg deg det dim exp gcd hom inf ker lg
  lim ln log max min sup limsup liminf injlim projlim Pr` — notably
  missing `sech`, `csch`, and every inverse of sec/csc/cot and every
  inverse hyperbolic form (`arcsinh`, `arcsech`, ...). MathQuill exposes
  `MQ.config({ autoOperatorNames: "..." })` to extend that list — use it
  to add the missing names rather than leaving them rendering as
  italic-letter juxtaposition.
  The real work is evaluator-side: `evaluateLatex()`'s `parseAtom`
  currently only recognizes a *single* Latin letter as an identifier
  (whether a variable, or — if followed by `(`/`\left(` — a
  user-defined function call via `funcs`). Multi-letter built-in names
  need a second lookup table (name → JS `Math.*` function, e.g.
  `sin`→`Math.sin`, `asin`/`arcsin`→`Math.asin`, `ln`→`Math.log`,
  `log`→`Math.log10`) checked before falling through to the
  single-letter path, with the same broadcasting support (via
  `broadcastUnary`) the rest of the evaluator already has for ranges.
