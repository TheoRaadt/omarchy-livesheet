---
name: livesheet-backend-gotchas
description: Known gotchas and verification guidance for livesheet's native Qt/QML backend (src/main.cpp, src/backend.cpp/.h, src/theme.cpp/.h, src/Main.qml) — pdflatex's exit code lying about whether a PDF actually got written, an Omarchy theme switch replacing colors.toml (not editing it) which breaks a naive QFileSystemWatcher, and why a polled QProcess needs its own independent timeout-and-kill guard. Also covers when QtTest alone is NOT sufficient and you need to build and run the real app instead. Use this skill whenever working on PDF/LaTeX export, live theme-following, or any polled/async QProcess in livesheet's backend.
---

# livesheet backend gotchas (src/backend.cpp, src/theme.cpp, src/Main.qml)

These are gotchas the existing backend code already had to solve —
know them before touching `backend.cpp`/`theme.cpp` rather than
rediscovering them from scratch.

## `pdflatex` can exit 0 while still failing to produce a PDF

`pdflatex -interaction=nonstopmode` can exit with status 0 even when
it skipped writing the output PDF entirely — it treats some LaTeX
errors as warnings rather than hard failures, by design (that's what
`-interaction=nonstopmode` is for: don't stop for human input on an
error). **Never treat a zero exit code as "the export succeeded."**
`Backend::exportPdf()`'s real success check is whether the output file
actually exists afterward — that's the pattern to follow for any
similar "shell out to a tool that can silently warn-and-continue
instead of failing" situation.

## An Omarchy theme switch replaces `colors.toml`, it doesn't edit it in place

This matters specifically for anything using a `QFileSystemWatcher`
(or similar) to detect theme changes live: a *replace* (unlink +
recreate, often through a symlink retargeting one directory up) drops
the old file from the watch list the instant it happens, because the
watcher was watching that specific inode/path, not "whatever file
ends up at this path." `Theme::reload()` has to clear and re-add every
watched path on *every* reload — including the nearest existing
*ancestor* directory, not just the leaf file — so a theme installed
later (a path that didn't exist yet at the previous watch-setup time)
still gets picked up without requiring an app restart. If you're
adding anything that watches a file for live changes, check whether
the thing that changes it replaces the file/symlink chain rather than
editing content in place — if so, a one-time `addPath()` at the leaf
is not enough.

## A polled `QProcess` needs its OWN timeout-and-kill guard, independent of the poll interval

If a recurring check shells out via `QProcess` (e.g. the Hyprland
window-rounding poll in `Theme`), give that specific process call its
own timeout-and-kill guard, separate from whatever interval schedules
the next poll. Otherwise one hung/stuck invocation of the external
command can block every subsequent poll from ever starting — the poll
scheduler fires on schedule, but each firing just queues up behind the
one that never finished. This is the general pattern for any
recurring external-process check in this codebase: a one-off UI action
(like `pdflatex` in `exportPdf`) can get away with a single blocking
call plus an explicit timeout, but anything that runs on its own
recurring schedule needs to run fully async with its own
timeout-and-kill guard — pick whichever of those two matches what
you're adding, not a third pattern.

## When QtTest alone is not enough

`bin/test` (QtTest, offscreen Qt platform) is the right tool for
`Backend`'s pure logic (JSON parsing, LaTeX conversion) and `Theme`'s
fallback behavior, and any change to `backend.cpp`/`theme.cpp` needs a
matching test there covering the new/changed behavior — not just a
passing build. But it's a safety net for the *logic*, not a substitute
for seeing the real thing happen: for the actual export flow (the
dialog opening, `pdflatex` really running end to end) or live
theme-following (a real `colors.toml` change actually being picked up
while the app is running), build and run the real app (`bin/build`,
then launch `build/livesheet`) and watch it happen for real. A passing
unit test here tells you the conversion logic is right; it doesn't
tell you the dialog actually opens, or that a real theme switch is
actually observed live.
