---
name: program-blocks
description: Data model, keybindings, and scope decisions behind TODO.md's "program blocks" feature in src/notebook.html
metadata:
  type: project
---

Shipped 2026-11-25: a real for/while/if/break/continue/return construct,
scoped to one level of nesting, per TODO.md's agreed v1 design. Full
detail (including the MathQuill-behavior findings that drove several
design choices) is in [[frontend.md's 2026-11-25 lessons entry]]
(`.claude/agents/frontend.md`) — this memory is the quick-reference
summary.

**Line type**: `"program"`. A bare program (no function signature) is
triggered by `{` on a pending line; a function-body program is `{` typed
on a math line showing exactly `f(x):=` (mirrors `[` for a named vs.
unnamed matrix). `line.statements` is the flat top-level sequence;
`for`/`while`'s own `body` and `if`'s `thenBody`/`elseBody` hold only
plain assign/return/break/continue statements — never another
structural statement (enforced on the UI side, in `insertStructural()`,
not in the interpreter).

**Keybindings** (none are real Mathcad conventions — PTC's own
Programming toolbar inserts these by click, confirmed before picking
anything): `:` inside a program statement's own field means `\leftarrow`
(not `:=` — reuses the existing physical key, new meaning in this
context) · `Ctrl+Shift+R` for-loop · `Ctrl+Shift+W` while-loop ·
`Ctrl+Shift+N` if/otherwise (not `I` — already the integral trigger) ·
`Ctrl+Shift+B` break · `Ctrl+Shift+T` continue · `Ctrl+Shift+E` return.
`for`/`while`/`if`/`otherwise`/`∈` are rendered as plain static-text
labels, never typed into a MathQuill field — sidesteps the whole
"does autoOperatorNames already cover these keywords" question from
TODO.md entirely, same pattern a graph line's "y =" label already used.

**Interpreter**: `runProgram()`/`runStatement()` — local-only assignment
scope (copies `vars` once at entry, never mutates the caller's object),
`break`/`continue`/`return` as thrown `{__programSignal: ...}` sentinels,
`PROGRAM_MAX_ITERATIONS = 100000` enforced on both `for` and `while`. A
program-backed function body is `funcs[name] = {params, program: line}`
(as opposed to `{params, body: latexString}`) — `evaluateLatex()`'s
function-call site checks for `.program` and dispatches to `runProgram()`
instead of recursing into itself.

**Comparison operators** (`<`,`>`,`=`,`\leq`,`\geq`,`\neq`) didn't exist
before this feature — added as a new `parseComparisonExpr()` precedence
level directly below `evaluateLatex()`'s entry point, binding looser
than +/-. Zero regression risk: a literal `=`/`<`/`>` never reaches
`evaluateLatex()` for any pre-existing line shape.

**Known, documented v1 limitations** (not oversights — see
`insertStructural()`'s/`locateStatement()`'s own comments): deleting an
entire `for`/`while`/`if` structure via Backspace isn't supported (only
a plain assign/return/break/continue row backspaces away); `for` only
accepts a range or a named matrix as its collection (a bare scalar, or
one row/column of a matrix, is not supported — `flattenCollection()`
reused as-is, not extended).

See [[symbolic-calculus-integration]] for the unrelated phase-2
indefinite-integration feature shipped the same day, and
[[feedback_live_verification]] for the CDP-driven live-test recipe this
feature's own verification built directly on (plus a new finding about
`.keystroke("Enter")` not working, documented in the frontend.md entry
above).
