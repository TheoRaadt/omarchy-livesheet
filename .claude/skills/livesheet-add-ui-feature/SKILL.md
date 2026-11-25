---
name: livesheet-add-ui-feature
description: Checklist and concrete traps for adding a new line type, keyboard shortcut, or MathQuill-field-level interaction to livesheet's src/notebook.html. Covers the "every line.type dispatcher needs a case" failure mode (which has caused a severe silent whole-app freeze before), the fresh-pending-line keystroke timing gap, never shadowing a key MathQuill itself already handles (Enter/deleteOutOf/moveOutOf) at the document level, checking a new Ctrl-modified shortcut's real browser default before deciding it's safe to leave unguarded, and the CSS custom-property cascade trap. Use this skill whenever adding a new line type (anything alongside math/matrix/graph/program), a new Ctrl or Ctrl+Shift keyboard shortcut, or any new structural field/UI to notebook.html — before writing the keydown handler or the dispatcher case.
---

# Adding a UI feature to livesheet (src/notebook.html)

These are the specific, previously-hit traps for this exact codebase —
not generic frontend advice. Each one caused a real bug (several
severe) in a previous session; read the "why" so you can recognize the
pattern in a new situation, not just avoid the literal example.

## 1. A new line type needs a case in EVERY dispatcher, not just the one you're working on

`src/notebook.html` has a handful of functions that switch on
`line.type` — at minimum `focusLine()`, `collectDocument()` (export),
and `collectDefinitions()`'s registration/skip logic — and there may
be more by the time you're reading this (grep for
`line.type ===`/`l.type ===`/`hostLine.type ===` across the whole
file, don't assume the list above is exhaustive).

**Why this matters more than it sounds like it should**: adding the
`graph` line type without adding it to `focusLine()`'s switch meant a
graph line fell through to the generic contenteditable-focus branch,
which does nothing on a non-editable container. The failure was severe
and silent — backspacing an empty graph line left focus nowhere at
all, and *every subsequent keystroke anywhere in the document* did
nothing, with no error or visual sign anything was wrong. A missing
dispatcher case doesn't always fail loudly or locally.

Whenever you add a new line type: grep for every `.type ===` site
first, add your case to each one, and test the new type specifically
at whatever behavior each dispatcher controls (focus, export,
definition-scanning), not just the one behavior you were building.

## 2. The "fresh pending line's first keystroke" timing gap generalizes to ANY new keydown-tracked feature

The global capture-phase keydown listener resolves a keystroke's
target field via `.closest(".math-line, .graph-line")` /
`.closest(".matrix-cell")` (or similar) — a brand-new `pending` line
(plain `contenteditable`, no MathQuill field yet) doesn't resolve to
anything through that path. An unmodified-character shortcut (`\`,
`;`, `$`, `#`, ...) doesn't notice, because it goes through a
*different* mechanism on a fresh line: the browser's own default text
insertion fires an `input` event, which `wirePendingInput()` catches
and replays through `insertChar()` once the field exists.

A **modifier-key** shortcut (`Ctrl+<letter>`, or any feature tracking
raw keystrokes via the global listener) does NOT get this for free —
holding a modifier suppresses the browser's normal insertion, so
nothing lands, no `input` event fires, and there's nothing for
`wirePendingInput()` to replay. This has independently bitten the
Greek-letter shortcut and the inline-completion tracker, for the exact
same root cause each time: **each new feature has its own tracking
state, so each one needs its own fix**, not just "the Greek-letter fix
already covers this." The established fix: when the normal resolution
comes back empty, fall back to `e.target.closest(".pending")._line`
and, if that line is still `type === "pending"`, call
`convertToMath(line, "")` yourself before acting on the field that
conversion just created.

## 3. Never shadow a key MathQuill itself already has a configured handler for

MathQuill's `handlers` option supports `enter`/`deleteOutOf`/
`moveOutOf` (among others) per field. If you're adding a new
structural keyboard shortcut and you're tempted to intercept that same
key at the document level (capture phase, which always runs before a
target's own listeners) — don't. Doing so makes the field's own
configured handler permanently dead code, because
`stopImmediatePropagation()` at the document level means MathQuill's
internal handling for that key never runs at all. This has already
happened once (an early draft of the program-block feature added a
redundant document-level `"Enter"` interception, which would have
killed `handlers.enter`).

Before adding a document-level interception for any key, check what
the existing precedent already does: `Tab` genuinely needs document-
level handling everywhere in this app (matrix cells, program fields)
because MathQuill has **no** configurable `"tab"` handler option —
but `Enter` never does, anywhere, because it does have one. If you're
unsure whether a key has a configurable handler, check what the
existing math-line/matrix-cell/graph-line fields already do before
adding a new interception for it.

## 4. Check a new Ctrl-modified shortcut's REAL browser default before deciding it's safe to leave unguarded

This app's convention for most shortcuts (Greek letters, the integral
trigger) is to call `preventDefault()`/`stopImmediatePropagation()`
only inside the branch where a usable target field actually resolved
— leaving the browser default to fire otherwise. That's a *deliberate,
accepted* tradeoff for those specific keys, where the browser default
is benign (opening DevTools, or nothing at all).

**That tradeoff is not safe to copy reflexively for every new
binding.** Before choosing it for a new shortcut, actually check what
that modifier+key combination's real browser/Chromium default is.
`Ctrl+Shift+R` is a hard-reload shortcut — and this app has zero
save/persistence (session-only, by design) — so leaving that one
unguarded when no target resolves would silently wipe the entire
in-progress notebook from a single stray keystroke anywhere else in
the document. When a combo's default is destructive (not just
cosmetic or a no-op), call `preventDefault()`/`stopImmediatePropagation()`
unconditionally the moment the modifier+key matches, before checking
whether a target resolves — there's essentially never a reason to
prefer preserving a destructive default over a silent no-op in this
app.

## 5. Before wiring a keyboard shortcut to any token, check whether the evaluator already treats that token specially

If the token your new shortcut inserts (a Greek letter, a new symbol)
is also one of `evaluateLatex()`'s own special leading tokens — the
kind `parseAtom()` checks for unconditionally before ever asking "is
this a defined variable" (e.g. `\Sigma`/`\Pi` for summation/product) —
trace that check order *before* wiring up the shortcut, not after. The
failure mode here doesn't throw: `\Sigma := 5` then `\Sigma + 1` would
silently parse as "reduce `+1` over nothing," giving a wrong-but-valid
number instead of an error you'd notice. This is a correctness check,
not a testing step — you can't catch it by testing the shortcut in
isolation, only by reading how the evaluator's atom-parsing actually
prioritizes tokens.

## 6. A page-wide CSS custom property belongs on `<html>`, not `<body>`

A CSS custom property cascades to descendants only, and `<body>` is a
descendant of `<html>`, not the reverse. A theme toggle (or any
page-wide visual state) that sets its class/variables on `<body>`
leaves `<html>`'s own background (which extends the full viewport,
usually taller than `<body>`'s content box) stuck on the old value
below wherever the content ends. If you're adding new page-wide
theming state, put it on `<html>`.

## 7. If a new structure has multiple branches, seed a starter row in EVERY branch, not just the primary one

When a new feature creates a structure with more than one body/branch
(e.g. an `if`/`otherwise` with a `thenBody` and an `elseBody`), make
sure *every* branch gets its own starter/seed content if any of them
need one to become reachable through the UI at all. Seeding only the
primary branch and assuming the user will somehow reach the other one
is an easy, easy-to-miss trap: if nothing in that branch has a focusable
field yet, and no other field's "add a row" action targets it, it can
end up permanently unreachable through the UI — not just empty, but
with no way to ever add a first row to it.
