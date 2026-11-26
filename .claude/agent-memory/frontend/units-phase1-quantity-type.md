---
name: units-phase1-quantity-type
description: Physical-units Phase 1 (quantity type + core arithmetic) — scope decisions that needed adjusting from the manager's spec, and the MathQuill-emission facts that drove them
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
