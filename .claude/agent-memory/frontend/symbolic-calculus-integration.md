---
name: symbolic-calculus-integration
description: Design decisions behind phase 2 (symbolic indefinite integration) of the symbolic calculus feature in src/notebook.html
metadata:
  type: project
---

Phase 2 of TODO.md's "symbolic calculus" item (indefinite/symbolic
integration) shipped 2026-11-25, built strictly on phase 1's
(`dc2e053`) AST layer in `src/notebook.html` — `parseLatexToTree`,
`containsVar`, `simplifyTree`, `treeToLatex`, `appendFormulaResult` all
reused as-is, zero changes to `evaluateLatex()`/`collectDefinitions()`.

**Trigger**: `Ctrl+Shift+I` inserts `\int` followed immediately by an
auto-paired `(`, rendering as `\int\left(f(x)\right)dx` (not bare
`\int f(x)\,dx`). This paren wrapper is NOT cosmetic — it's required
because a bare `\int` symbol has no block boundary, so MathQuill's own
auto-fraction grab-back would otherwise swallow the `\int` symbol
itself into a numerator the moment the integrand contains a `/`
(confirmed live: `\int` then `1/x` with nothing between produces
`\frac{\int1}{x}`, not `\int\frac{1}{x}`). `matchIntegralTrigger()`
detects the shape purely textually: must start with `\int`, must end
with literal `d` + one identifier token and nothing after.

**Why `Ctrl+Shift+I` and not `Ctrl+I`**: Mathcad's own real convention
(PTC docs) is version-dependent — legacy Mathcad uses `Ctrl+I` for the
indefinite integral, later Prime-era Mathcad uses `Ctrl+Shift+I`
generally. `Ctrl+I` is already claimed in this app by the Greek-letter
feature (`\iota`), so `Ctrl+Shift+I` was used — both free and itself a
genuine Mathcad convention, not an arbitrary pick.

**Scope** (deliberately bounded, see TODO.md and this feature's own
code comments for the full rationale): power rule incl. n=-1→ln(x),
sin/cos/exp, constant-base exponential (e^x special-cased to skip the
ln(e) factor), linearity, constant-factor pull-out, and "simple
composition" ONLY via linear substitution f(ax+b) — never general
u-substitution. `\tan`/`\ln`/`\log` are explicitly excluded, not just
unimplemented. A term the engine can't handle throws uncaught through
every recursive call, so the whole integral fails gracefully (same
"leave it alone" UX as a numeric line with an undefined variable) —
never a partially-integrated result.

See [[frontend.md's 2026-11-25 lessons entry]] (`.claude/agents/
frontend.md`) for the concrete MathQuill-behavior findings that drove
these choices, and `PROGRESS.md` for the verification record.
