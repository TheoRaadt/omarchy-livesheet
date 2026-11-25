---
name: feedback-live-verification
description: How to live-drive src/notebook.html for verification when claude-in-chrome isn't connected, and the insertChar("=") gotcha that trips up hand-written test scripts
metadata:
  type: feedback
---

When claude-in-chrome (`mcp__claude-in-chrome__*`) reports "Browser
extension is not connected", don't fall back to `wtype` against the
compiled Qt app (project memory already flags that as unreliable and
once unsafe) and don't give up on live DOM verification either. Serve
`src/` with `python3 -m http.server`, launch `chromium --headless=new
--no-sandbox --disable-gpu --remote-debugging-port=<port>
--remote-allow-origins=* about:blank` in the background, and drive it
directly over the DevTools Protocol using Node's native `WebSocket`
global (Node 21+ has it built in — no puppeteer/playwright install
needed, which is good because this sandbox's permission classifier
blocks ad-hoc `pip install`/npm-global-install anyway). Full working
recipe, including the "PUT not GET for /json/new" trap, is written up
in `.claude/agents/frontend.md`'s dated 2026-11-25 entry — read that
before re-deriving this.

**Why this matters**: `notebook.html`'s single `<script>` block has no
IIFE wrapper, so every top-level function/var (`lines`,
`createPendingLine`, `convertToMath`, `insertChar`, `evaluateLatex`,
`renderGraph`, ...) is a real global — `Runtime.evaluate` can call the
app's actual internal functions directly, which drives the real
MathQuill field and real evaluator exactly as a live keystroke would,
not a Node-level copy of the logic. This is strictly better than the
Node-copy-of-the-functions test when the question is "does the real
DOM/MathQuill state behave as expected" — do both, per the project's
own verification standard, but the DOM-level one needs this fallback
when claude-in-chrome is down.

**How to apply**: `insertChar(line, "=")` is what actually evaluates
and appends a result — NOT `mathField.typedText("=")`. Calling
`typedText("=")` directly bypasses `insertChar`'s special-casing (the
`ch === "="` branch in `src/notebook.html`) and silently inserts a bare
"=" with no evaluation, which looks exactly like an app bug in a test
script if you don't trace `insertChar`'s branches first. Always type
through `insertChar` (or replay a whole buffered string one character
at a time through it, same as `convertToMath()`'s own replay loop) when
a test needs the real "=" trigger, "@" trigger, etc. — plain
`typedText`/`cmd` calls are fine only for the literal content between
special characters.
