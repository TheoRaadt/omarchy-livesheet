---
name: livesheet-manage-project
description: Concrete practices for the manager session coordinating livesheet's frontend/backend subagents — how to tell a scope-bounding decision (yours to resolve before delegating) apart from a genuine product fork in an underspecified request (the user's to resolve, via AskUserQuestion, not yours to silently pick), and why README.md needs its own deliberate staleness check separate from TODO.md/PROGRESS.md's own upkeep. Use this skill before delegating any feature with real design surface, and periodically when reviewing the project's own documentation for drift.
---

# Managing livesheet (the coordinating/manager session)

These are concrete practices this project's own history already
proved out — not generic project-management advice. Each one is here
because a real session either got it right and it mattered, or had to
learn it the hard way.

## Three kinds of open question when picking up a request — only one is yours to resolve alone

Before delegating a feature with real design surface, sort every open
question into exactly one of these:

1. **Scope-bounding / risk-bearing decisions** — "how do we build what
   was asked for, safely." These are the manager's call: resolve them
   explicitly in the delegation prompt as non-negotiable constraints,
   don't leave them open for the subagent to guess at. Example: "a
   program block is a function body, reusing the existing
   `name(params):=` machinery, not a parallel mechanism."
2. **Empirical / UX discovery** — questions that need live discovery
   to answer at all ("what does MathQuill actually render for this
   keystroke," "is this name in `autoOperatorNames`"). These belong to
   the subagent — it's the efficient path, and second-guessing it from
   the manager's chair without running anything would just be
   guessing twice.
3. **A genuine product fork, because the ask itself was
   underspecified** — this is the one easy to misfile as category 1.
   Picking up two vague backlog items this project actually hit
   ("let `for` iterate one row/column of a matrix," "revisit the
   nesting cap") turned up that each had a materially different-sized
   version with real, lasting consequences either way (a full general
   matrix-indexing feature vs. a narrow for-loop-only helper; fully
   general nesting vs. one extra level only) — and "add dimensions to
   the project" turned out to mean anything from a plain named-
   constant shortcut to full SI dimensional analysis touching nearly
   every operator in the evaluator. Don't silently pick the smaller
   reading (a dead end once the user actually wanted more) or the
   larger one (unrequested scope, unrequested cost) — ask, with
   `AskUserQuestion`, a recommended default, and concrete before/after
   examples if the options are hard to compare in the abstract (a
   `preview` block showing `5m + 3s → error` vs. `5m + 3s → 8` made the
   units-depth question legible in a way prose alone wouldn't have).
   The tell that you're in category 3, not category 1: the different
   readings don't just cost different amounts of work, they produce
   genuinely different end-user capability — that's a decision about
   *what was asked for*, not *how to build it*.

For an unusually large feature (bigger than anything built so far),
consider also splitting the actual delegation into phases once scope
is settled — build the core architecture end-to-end against a small,
real proof-of-concept data set first, review that, THEN expand the
data/content in a separate follow-up task. Physical units' Phase 1
(core quantity type + 10 units, validating every operator) vs. Phase 2
(expanding to the full practical unit list, a pure data-entry task
once Phase 1's architecture is reviewed) is the clearest example — it
mirrors how symbolic calculus was split into derivatives then
integrals. A large feature built as one giant delegation is harder to
review and riskier to get wrong in a way that's expensive to unwind.

## `README.md` drifts silently — it needs its own deliberate check, not just TODO.md/PROGRESS.md's

`TODO.md`/`PROGRESS.md` get updated reliably because they're directly
tied to the work a session just did — closing out a backlog item
naturally touches both. `README.md` doesn't have that same forcing
function: it only gets touched when a session happens to notice it's
stale. Found concretely (2026-11-26): `README.md` had no mention at
all of several matrix functions (`det`/`trace`/`^T`/`^-1`/`solve`/
`mean`/`median`/`variance`/`stdev`) that had been shipped and verified
in `PROGRESS.md` across multiple earlier sessions, and separately
still described a Greek-letter keybinding bug as open ("Fix queued,
see `TODO.md`") that had actually been fixed and live-verified two
sessions earlier, with `TODO.md` itself no longer having any such item
to point to.

When picking up work that touches a user-facing feature (not just
closing out `TODO.md`'s own backlog), spot-check whether `README.md`
actually still describes current behavior — grep it for the feature
area you're touching, and for any `"see TODO.md"`/`"known
limitation"`-style pointer, confirm the thing it points to still
exists. Don't assume past sessions kept it in sync just because
`TODO.md`/`PROGRESS.md` look current. Worth an occasional deliberate
full pass (README against PROGRESS's "Done" section), not only a
reactive one triggered by writing up a brand-new feature.
