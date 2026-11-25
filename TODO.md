# TODO

Running backlog of requested features, tracked by the manager session
so a new session can pick up immediately without re-reading the whole
conversation history. Update status here as items move; once something
is actually done and verified, its details belong in `PROGRESS.md`, not
here — this file should only ever describe what's *not* done yet, plus
enough design context that whoever picks an item up next doesn't have
to re-derive decisions already made.

Last updated: 2026-11-25 (`e`/`π` built-in constants, scalar-matrix
multiplication, and symbolic derivatives shipped — see `PROGRESS.md`).
Everything below this line is still open.

## Queued — symbolic integrals (phase 2 of symbolic calculus; build on phase 1's AST, don't re-derive it)

Phase 1 (symbolic **derivatives**) shipped 2026-11-25 — see
`PROGRESS.md`'s entry for the full design. This item is specifically
the **indefinite-integral/antiderivative** half the user asked for
when choosing "full symbolic antiderivative" over numeric
finite-differences/quadrature — re-scoped 2026-11-24, and derivatives
were pulled in first "for consistency" at the time, but have now
actually shipped first since they're the simpler direction (a fixed
set of differentiation rules vs. the open-ended "which technique even
applies" problem every real integration engine has to solve).

**Don't redesign the AST — reuse phase 1's exactly as built**, in
`src/notebook.html`: the node shapes (`num`/`var`/`neg`/`add`/`sub`/
`mul`/`div`/`pow`/`call`), `parseLatexToTree()` (parses a self-contained
LaTeX expression, no `vars`/`funcs` lookup), `simplifyTree()` (small
identity-cleanup pass, deliberately not a general simplifier — same
scope limit applies to integral results), `treeToLatex()` (renders a
tree back to display LaTeX), and `appendFormulaResult(line, latex)`
(the generic "formula result" DOM widget, sibling of the line's own
field, read-only `MQ.StaticMath` — built generically specifically so
this item wouldn't need its own display mechanism). All of these are
plain top-level functions in `src/notebook.html`, right after
`formatNumber()` — grep for `parseLatexToTree` to find the whole
section.

**Trigger syntax — not yet decided for integrals, this delegation's
job**: derivatives settled on typed Leibniz notation (`d`, `/`, `d`,
`<var>` → MathQuill's own `\frac{d}{dx}` — confirmed live, see
`matchDerivativeTrigger()`). The analogous integral notation is
`\int f(x)\,dx` (an integral sign, the integrand, then `dx`) — but
**confirm live, don't assume, how a user actually types an integral
sign in this app first**: grep for any existing `\int` handling/
keyboard shortcut in `src/notebook.html` (there was none as of phase
1's work finishing), and check whether MathQuill's default typed-`\int`
behavior (if typeable at all via raw LaTeX command entry) produces
usable sub/superscript slots that need to be left empty for an
*indefinite* integral, or whether a dedicated shortcut needs adding
(this project already has a precedent for adding one, e.g. `Ctrl+P` for
`\pi`, `Ctrl+B` for Greek — check `.claude/agents/frontend.md` and the
Greek-letter section of `PROGRESS.md` for that pattern before inventing
a new one). Whatever trigger-detection function results from this
(e.g. an `matchIntegralTrigger()` alongside `matchDerivativeTrigger()`)
should follow the same "return null to fall through to the ordinary
numeric path" convention `matchDerivativeTrigger()` already
established, checked at the same point in `evaluateAndAppend()`.

**Agreed v1 scope** (same as derivatives, not to be expanded without
checking back): polynomials (power rule, i.e. `∫xⁿdx = xⁿ⁺¹/(n+1)` for
`n ≠ -1`), trig, exp/log, and simple compositions via substitution —
but note integration is NOT just "run differentiate() backwards";
genuinely different technique selection (which rule even applies to a
given integrand) is required, this is real integration-engine design
work, not a mechanical inverse of phase 1. Keep it bounded to what a
calculator-grade tool's integral button needs (the standard-forms
table plus u-substitution for compositions matching something already
in that table), not a general symbolic-integration system (no
integration by parts, no partial fractions, no trig-substitution,
unless a future request asks for more). An integrand this scope can't
handle should fail gracefully (leave just the typed expression,
same "can't evaluate yet" UX the numeric path already has for an
undefined variable) rather than throwing a confusing error or
guessing.

**Verification standard, same as everything else in this project**:
both a throwaway Node-level logic test AND live DOM verification
(prefer `claude-in-chrome`; the documented DevTools-Protocol-via-Node-
`WebSocket` fallback in `.claude/agents/frontend.md`'s 2026-11-25
entries is proven to work when that extension isn't connected). Also
re-run `bin/build`/`bin/test` and spot-check a few ordinary (non-
calculus) lines still work — phase 1 was built strictly additively
with zero changes to `evaluateLatex()`/`collectDefinitions()`, and
phase 2 should hold to that same discipline.

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

