---
name: units-phase1-quantity-type
description: Physical-units Phase 1 (quantity type + core arithmetic) and Phase 2 (full unit list) — scope decisions that needed adjusting from the manager's spec, and the MathQuill-emission facts that drove them
metadata:
  type: project
---

Built TODO.md's units feature, Phase 1 (core `{__quantity}` type + a
small proof-of-concept unit set: m/s/A/K/N single-letter, kg/mol/cd/Pa/Hz
multi-letter — see [[livesheet-extend-evaluator]]/[[livesheet-live-verify]]
for the general techniques used). The manager's spec was unusually
detailed and got almost everything right; three places needed a real
adjustment, not just "my judgment on an open detail":

1. **`\frac` needed an opName too, not just `\div`.** The spec's enumerated
   call-site list (parseExpr's +/-, parseTerm's `\div`, parsePow's `^`,
   `\sqrt`'s index) never mentioned `\frac` at all. But a typed `/` key
   always produces `\frac{}{}` in this MathQuill build — `\div` is a
   separate, basically-unreachable LaTeX token nothing in this app's
   keybindings ever types. Without adding `"/"` to the `\frac` call site
   too, EVERY required verification case that divides two quantities
   (`2kg*9.8(m/s^2)`, `10m/2s`, `(5*m)/m`) would have silently fallen
   through to plain-JS-object division instead of the new dims logic.
   Caught immediately by tracing "how does a user actually type division"
   before writing any live test, not by the live test itself.
2. **Un-indexed `\sqrt` (no explicit root index) was left out of the
   call-site list too**, and unlike `\frac` this one doesn't break a
   required verification case — it's a genuine gap beyond the spec's
   literal scope. `broadcastUnary()` (what un-indexed `\sqrt` used before)
   passes a quantity's `dims` through UNCHANGED, only transforming the
   value — correct for unary minus, silently WRONG for sqrt (`\sqrt{m^2}`
   would've rendered "m^2", not "m"). Fixed by routing the un-indexed
   case through the same `broadcast(..., 0.5, ..., "^")` path the
   explicit-index case already needed, rather than leaving it on
   `broadcastUnary()`. Confirmed both live and in Node: `\sqrt{m}` now
   correctly throws "fractional unit exponent not supported" (length:1
   isn't evenly halvable) and `\sqrt{m^2}` correctly gives dims length:1.
3. **The `edit` handler on a math line's MathQuill field** (inside
   `convertToMath()`) already cleared a stale `resultMatrixEl`/
   `resultFormulaEl` on re-edit, but had no equivalent for the new
   `resultQuantityEl` — found by reading the surrounding code before
   wiring up `appendQuantityResult()`, not by hitting the bug live. Fixed
   by adding the same `if (line.resultQuantityEl) {...}` cleanup line.

**Design choice confirmed safe via the established invariant**: once
`collapseIfDimensionless()` always runs at the end of every quantity-
producing path, a `{__quantity}` object that survives ANYWHERE in this
file always has at least one nonzero `dims` component — so every
"is this a quantity with real units" check downstream (comparisons, `^`'s
exponent check, `callBuiltin()`'s dimensionless-arg guard) only needs
`isQuantityValue(x)`, no separate all-zero check. Worth remembering if
Phase 2 adds a new quantity-producing path: it MUST end with
`collapseIfDimensionless()` too, or this invariant (and everything that
relies on it) silently breaks.

**`parseComparisonExpr()`'s `broadcast()` calls are the ONE place in the
whole grammar that still passes no `opName`** (by design, both before and
after this feature) — `quantityBroadcast()` treats `opName === undefined`
specifically as "this is a comparison, not arithmetic". This is safe
specifically because every other call site now explicitly passes one.

See the dated bullet in `.claude/agents/frontend.md` for the
MathQuill-specific empirical details (the `\frac` vs `\div` finding, the
`^` single-char-exponent-adjacent mechanics). This file is the "why/
scope" record; that one is the "concrete trap" record.

## Phase 2 (2026-11-27): expanded UNIT_TABLE, pure data entry, two findings

Added ft/in/mi/cm/mm/km/lb/oz/hr/gal/Wb (multi-letter) and g/J/W/V/F/T/
H/C/`\Omega` (single-letter, shadowable) to `UNIT_TABLE`, plus the
matching symbols into `NAMED_DERIVED_SYMBOLS`. The arithmetic/display
machinery genuinely needed zero changes, as the manager's spec claimed
— except for one small, necessary addition (see finding 2 below), which
is display-only, not arithmetic.

1. **Dropped "min" (minutes) from the candidate list entirely.**
   MathQuill's `autoOperatorNames` already auto-recognizes "min" as the
   native minimum-function operator — typing the three characters "m",
   "i", "n" produces the single atomic LaTeX token `\min` in
   `.latex()`, not the literal substring "min" this file's
   literal-substring unit-matching mechanism requires. Confirmed by
   actually typing it into a throwaway MathField and reading back
   `.latex()` (`10\cdot\min`), not assumed. Same class of gotcha as the
   already-documented "asin" case, just for a would-be unit symbol
   instead of a function name. Fixing it properly would need a
   `parseAtom` special case for the bare `\min` token — a real grammar
   change, out of this phase's "pure data entry, no machinery changes"
   scope — and would also permanently block ever giving `\min`/`\max` a
   real min/max-function meaning later. "hr" alone covers the time
   category for now; a future session revisiting minutes should
   consider a different symbol (not "min") rather than re-fighting this.
2. **Ω's display label needed a tiny new lookup, `UNIT_LABEL_DISPLAY`.**
   `appendQuantityResult()`'s unit-label span is plain `textContent`,
   never MathQuill. Every ASCII unit symbol is already its own correct
   display text, so this was never a problem before — but Ω's `symbol`
   has to be the LaTeX command-token string `"\Omega"` (there's no
   Latin-letter form, and that's the exact token `matchIdentifierAt()`
   already recognizes via the Greek-letter machinery). Left alone, that
   literal string renders as the text "\Omega", not the Ω glyph —
   confirmed live (typed `\Omega=` and `V/A=` into the real page) before
   trusting the code reading. Fixed with a one-entry `symbol ->
   display-text` map consulted ONLY at the `textContent =` assignment
   site, not by normalizing the token earlier — every other place that
   keys on `quantity.displayUnit`/`findNamedUnitForDims()` (seed
   lookup, dims matching) correctly wants the literal `"\Omega"` token,
   so normalizing it away upstream would have broken that.

General lesson for any future phase that adds a unit symbol: (a) always
actually type a new multi-letter candidate into a throwaway MathField
and read back `.latex()` before trusting it survives as literal
characters — don't just reason "it's a short lowercase word, it's
probably fine" the way "min" looked fine on paper; (b) if a new
symbol's natural form is a command token rather than its own literal
display text (anything non-Latin-letter), grep for every place a
table's `symbol` field gets rendered as plain text, not just where it
gets matched — matching code and display code can have different
assumptions about what that string "is."
