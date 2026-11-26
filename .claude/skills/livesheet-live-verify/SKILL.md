---
name: livesheet-live-verify
description: How to live-verify a change to livesheet's src/notebook.html by actually driving the real MathQuill-based page with synthetic keystrokes and reading back the result, instead of trusting a Node-level copy of the logic alone. Covers the preferred tooling order (claude-in-chrome, then a documented DevTools-Protocol fallback, wtype against the compiled Qt app only for a genuinely native-level question like "does something intercept this key before notebook.html's own JS"), this sandbox's non-obvious hyprctl dispatch syntax and the safe wtype-Alt+Tab focus workaround, and the specific MathQuill/CDP gotchas that make a live test look like a false app bug if you don't know them. Use this skill whenever testing or verifying ANY change to src/notebook.html — a new keyboard shortcut, a new line type, a new evaluator rule — before claiming a change works, and especially whenever claude-in-chrome reports "Browser extension is not connected."
---

# Live-verifying livesheet (src/notebook.html)

## Why this exists

Every real bug ever found in livesheet's frontend (`src/notebook.html`)
was caught by driving the actual running page with real keystrokes —
none were caught by a Node-level copy of the evaluator logic alone,
and none would have been caught by code review. A pure-function copy
of the evaluator only proves the *algorithm* is right; it says
nothing about whether MathQuill actually produces the LaTeX the
algorithm assumes, or whether the DOM wiring (focus, event handlers,
CSS cascade) connecting a keystroke to that algorithm is correct. This
project's evaluator logic has consistently been right on the first
attempt while the DOM wiring around it consistently wasn't — so for
any change touching MathQuill, line types, keyboard shortcuts, or CSS
theming, do **both**: a throwaway Node-level test of the pure logic
(fast, precise, catches algorithm bugs), and a real live test per this
skill (catches everything else, and is the one that's caught every
bug so far).

## Tooling order — try these in order, don't skip to the last one

1. **`claude-in-chrome`**, driving a copy of `src/` served over HTTP
   (`python3 -m http.server` from the `src/` directory). Real keyboard
   events, real DOM/console inspection, and doesn't touch the compiled
   Qt app at all.
2. **If `claude-in-chrome` reports "Browser extension is not
   connected"**: don't give up on live DOM verification, and don't
   fall back to `wtype` against the compiled app either (see why,
   below). Use the DevTools-Protocol fallback in the next section —
   it needs no extra dependencies.
3. **`wtype` against the compiled Qt app is a last resort, not a
   default** — it has caused a real safety incident in this project:
   `hyprctl`'s focus-switch call reported success while the monitor's
   actual focus silently stayed on the terminal running the Claude
   Code session itself, sending synthetic keystrokes into that
   session's own chat input instead of into livesheet. It's also been
   observed to silently drop a typed `-` character, and the terminal
   running Claude Code can regain window focus mid-test and swallow
   keystrokes with no error. None of this is a livesheet bug — it's
   this specific Hyprland/Wayland environment — but `notebook.html` is
   pure static HTML/JS with no build step, so there is no actual
   reason to touch the compiled app for a frontend-only change. Reach
   for the compiled app only when the thing under test genuinely needs
   the Qt/QML shell (the export dialog, theme-accent reading, native
   window chrome, **or whether something NATIVE — not notebook.html —
   intercepts a given key before the page's own JS ever sees it**, e.g.
   confirming `F1` isn't claimed by some Qt/QML shortcut or a
   Chromium-core accelerator the way `Ctrl+Shift+R`/hard-reload is —
   this specific question is unanswerable against a plain
   `notebook.html` copy in a regular browser tab, since that's a
   different host entirely).

   **If you do reach for it, this environment's `hyprctl dispatch`
   needs its own non-obvious syntax** — plain classic dispatcher
   strings (`hyprctl dispatch focuswindow title:livesheet`) fail with a
   Lua parse error here; this sandbox's Hyprland wraps `dispatch` in a
   Lua layer. Don't burn time guessing `hl.dsp.*` call forms. Instead:
   launch the app while the monitor's already-active workspace is the
   one you want it on (a fresh window with no matching window rule
   lands there by default, no dispatch needed), then focus it with a
   **physical** keypress instead of any `hyprctl` focus/workspace
   dispatch: `wtype -M alt -k Tab -m alt` (Alt+Tab, the native
   `ALT+TAB` keybind already wired in) — confirm it worked via
   `hyprctl activewindow -j` before sending anything further. This is
   also the fix for the focus-stealing incident above: **never trust a
   dispatch/focus call's own reported success — always confirm via an
   independent query (`hyprctl activewindow -j`) AND a `grim`
   screenshot showing the actual expected content landed in the actual
   livesheet window**, not just that some command returned 0. A
   screenshot showing the right rendered math is strictly stronger
   proof than any API return value, and is what actually catches a
   silent focus-steal if one happens anyway.

## The DevTools-Protocol fallback (when claude-in-chrome isn't connected)

No puppeteer/playwright install needed — Node 21+ has a native
`WebSocket` global, which is all this requires.

1. Serve the directory: `python3 -m http.server <port>` from `src/`.
2. Launch headless Chromium in the background:
   `chromium --headless=new --no-sandbox --disable-gpu
   --remote-debugging-port=<cdp-port> --remote-allow-origins=*
   about:blank`
3. Open a tab: `PUT http://localhost:<cdp-port>/json/new?<url>` —
   **PUT, not GET**. A GET returns a plain-text "unsafe HTTP verb"
   error instead of JSON, which then throws a confusing `JSON.parse`
   error one layer removed from the real cause if you don't notice the
   error string first.
4. Connect to the tab's `webSocketDebuggerUrl` and wait for
   `Page.loadEventFired` before evaluating anything.
5. Drive the page via `Runtime.evaluate`. `notebook.html`'s `<script>`
   block has **no IIFE wrapper**, so every top-level function/var
   (`lines`, `evaluateLatex`, `createPendingLine`, `convertToMath`,
   `insertChar`, `renderGraph`, ...) is a real global — you can call
   the app's actual internal functions directly, which drives the real
   MathQuill field and real evaluator exactly as a live keystroke
   would. This is strictly better than a Node-level copy for "does the
   real DOM/MathQuill state behave as expected," but still isn't a
   substitute for the Node-level test of the pure logic — do both.

If you need to debug why a page failed to load at all under this
recipe (not just "a function threw"), note that `--dump-dom` does
**not** surface console/uncaught-error output — a missing-dependency
crash during page setup just looks like "the page didn't load," not
like an error. Re-run with `--enable-logging=stderr --v=1` and grep
stderr for `"CONSOLE"` to see what actually happened in-page.

## Gotchas that will make a correct test look like an app bug

- **`insertChar(line, "=")` is what actually evaluates and appends a
  result — `mathField.typedText("=")` is NOT.** Calling `typedText`
  directly bypasses `insertChar`'s own `ch === "="` special-casing and
  silently inserts a bare `"="` with no evaluation. Always drive a
  special trigger character (`"="`, `"@"`, etc.) through `insertChar`
  (or replay a whole string through it one character at a time, the
  same way `convertToMath()`'s own buffered replay does) — plain
  `typedText`/`cmd` calls are fine only for literal content between
  special characters.
- **A real `Enter` keystroke cannot be simulated with
  `.keystroke("Enter")`, and often not with a JS-constructed
  `KeyboardEvent` either.** `.keystroke("Left")`/`.keystroke("Right")`
  work fine through MathQuill's own API, but `.keystroke("Enter")`
  silently does nothing — confirmed even on an already-working,
  ordinary math line, not just a custom field. The real mechanism
  needs a genuine CDP `Input.dispatchKeyEvent` with `type:"keyDown"`
  (not `"rawKeyDown"` — that variant never fires the matching
  keypress/input Chromium normally synthesizes for a character key)
  plus `windowsVirtualKeyCode`/`nativeVirtualKeyCode: 13` and
  `text`/`unmodifiedText: "\r"`. A plain `document.dispatchEvent(new
  KeyboardEvent(...))` also fails for this one key even with
  `which`/`keyCode` passed to the constructor, because those two
  properties are getter-only on a real `KeyboardEvent` in modern
  Chromium — constructor values for them are silently ignored — and
  this bundled MathQuill build's own internal listener reads
  `e.which`/`e.keyCode`, not the modern `e.key`.
- **A real `Ctrl+<letter>` keydown CAN be simulated**, just not with
  `.keystroke()`: construct a `KeyboardEvent` with `ctrlKey: true` and
  the right `key`, override its `target` via
  `Object.defineProperty(evt, "target", {value: el})`, then
  `document.dispatchEvent(evt)` — this app's own shortcut listener is
  on `document` in capture phase and reads `e.key`, so it sees the
  event regardless of what it was dispatched from. This only works
  because the listener reads `e.key`; it's a different mechanism than
  the Enter case above (whose listener reads `e.which`/`e.keyCode`) —
  don't assume one trick covers both.
- **Any throwaway test page that loads `mathquill.min.js` must load
  `jquery.min.js` first**, exactly as `src/notebook.html`'s own
  `<head>` does. This bundled MathQuill build depends on jQuery and
  throws partway through its own top-level setup if `$`/`jQuery` isn't
  defined yet — before it ever sets `window.MathQuill` — so the
  failure looks like "MathQuill is not defined" / a script-loading
  problem, not a missing dependency, unless you check for it directly.
- **Don't trust the minified `mathquill.min.js` source to tell you
  what function names it auto-recognizes** (`autoOperatorNames`, e.g.
  whether `sech`/`csch`/`arcsec` render upright automatically). Its
  startup code expands a short hardcoded list (`sin cos tan sec cosec
  csc cotan cot ctg`) into every `arc`-/`h`-/`ar`- combination
  *programmatically at load time*, so the real default table is far
  larger than the literal strings visible in the source. Verify
  empirically instead: load the project's real `mathquill.min.js` (and
  `jquery.min.js` first) in a throwaway page, create a `MathField`,
  call `.typedText("name(2)")`, and read back `.latex()`. Also note
  `MQ.config({autoOperatorNames: "..."})`'s setter **replaces** the
  whole table with only the names given — it does not extend the
  default list, so "adding" a short string of just the names you think
  are missing would silently drop every other default name (`gcf`,
  `Pr`, `injlim`, ...).
- **For a feature with no single keybinding that assembles it end to
  end** (program blocks' statement trees are the clearest example — a
  real `for`/`while`/`if` body is built from several separate
  keystrokes: `Ctrl+Shift+R`, typed field content, `Enter` for each
  sibling statement, ...), building the test tree directly via the
  app's own internal constructors (`createStatementObject()`,
  `createProgramField()`, `runStatementList()`, etc. — real globals,
  per the no-IIFE note above) and then running the REAL interpreter/
  evaluator/MathQuill fields against it is a legitimate, faster middle
  ground between a full keystroke-driven UI test and a Node-level
  logic copy — it still exercises the real interpreter and real
  MathQuill fields, it just skips reproducing the UI-insertion
  keybinding sequence itself. Only safe when that keybinding sequence
  isn't what's actually under test (it wasn't, for testing a `for`
  loop iterating `row()`/`col()` — the insertion keybindings were
  unrelated, already-shipped code).
- **Test a brand-new line's very first keystroke separately from "a
  line that already has one character typed."** A modifier-key
  shortcut (`Ctrl+<letter>`) behaves differently on a genuinely fresh
  `pending` line than an unmodified-character shortcut does, because
  holding Ctrl suppresses the browser's normal text insertion — no
  character lands, no `input` event fires, so the buffered-replay path
  plain-character shortcuts rely on never runs for a Ctrl combo. Don't
  assume a shortcut that works after typing-then-backspacing a
  throwaway character also works as the literal first keystroke; test
  both cases explicitly.
