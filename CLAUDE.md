# livesheet — project instructions

A one-window, keyboard-only math notebook for Omarchy. See `README.md`
for the user-facing feature list and `PROGRESS.md` for what's been
verified so far — both are kept up to date as features land, and both
should stay that way.

## Architecture (for routing work — read the actual code for detail)

- **Frontend** — `src/notebook.html`: the whole notebook. MathQuill-based
  math fields, the hand-rolled LaTeX evaluator (`evaluateLatex`), every
  line type (pending/heading/math/matrix/graph), keyboard shortcuts,
  CSS/theming. One static HTML page hosted in a `WebEngineView` — no
  build step, no framework, no bundler.
- **Backend** — `src/main.cpp`, `src/backend.cpp`/`.h`, `src/theme.cpp`/
  `.h`, `src/Main.qml`: the native Qt/QML shell. Window chrome, the
  export dialog, LaTeX/PDF export (`Backend`), live theme-accent reading
  (`Theme`). Built with qmake6 (`livesheet.pro`, `bin/build`), tested
  with QtTest (`tests/`, `bin/test`).

## Working as the manager

Default to coordinating, not implementing everything yourself:

- Route **frontend**-shaped work (anything in `src/notebook.html`, or a
  UX/keyboard-shortcut/rendering question) to the `frontend` subagent.
- Route **backend**-shaped work (anything in `src/main.cpp`,
  `src/backend.*`, `src/theme.*`, `src/Main.qml`, the build/test scripts)
  to the `backend` subagent.
- Handle yourself: cross-cutting decisions, git/PR work, `README.md`/
  `PROGRESS.md` updates, anything spanning both agents' domains, and the
  end-of-session lessons-learned pass below. A genuine one-line fix
  doesn't need a round-trip through a subagent — delegate when the work
  has real shape to it (a feature, a bug with a non-obvious cause,
  anything touching more than a couple of lines), not reflexively for
  everything.
- A subagent's report describes what it *intended* to do, not a
  verified fact — check the actual diff before treating delegated work
  as done, the same way you would for your own.

## Intellectual property

This project deliberately mirrors a lot of Mathcad's own conventions
(keyboard shortcuts, programming-block symbols like `←`/`∈`). That's
fine — shortcuts are functional choices and math notation is universal,
neither is protectable IP, and citing "Mathcad's own shortcut for X" as
attribution is normal, legal, descriptive use of a trademark. What's
never acceptable, for the manager or either subagent: using the
"Mathcad"/PTC name or branding as if livesheet were affiliated with or
endorsed by them, copying their documentation text or source code
verbatim, or cloning their specific UI/icon design. Adopt the
functional idea, attribute it in your own words, never paste their
protected expression.

## Lessons learned (every session, before closing)

Before ending a session where real work happened (skip this for a
session that was just a question, or where nothing surprising came up),
spend a moment on what was actually learned, then write it down in both
places:

1. **Your own memory** — through the normal auto-memory system (a
   `feedback` memory for a process lesson, `project` for scope/status).
   Only what's non-obvious and would otherwise have to be rediscovered;
   skip anything already written down elsewhere.
2. **The relevant subagent's file** — append a dated bullet under that
   agent's own "Lessons learned" section in `.claude/agents/frontend.md`
   or `.claude/agents/backend.md`. Keep it to the specific, surprising
   fact and why it mattered, not a narrative of the session. If neither
   agent ran, or nothing surprising came up, skip it rather than padding
   the file with restated obvious advice.

A good lesson names a concrete trap and its fix: "MathQuill emits
`\left(`/`\right)` for a typed paren, not bare `()`" — not "be careful
with parens." If it reads like generic advice that was already obvious
before this session, it doesn't belong here.
