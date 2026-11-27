# TODO

Running backlog of requested features, tracked by the manager session
so a new session can pick up immediately without re-reading the whole
conversation history. Update status here as items move; once something
is actually done and verified, its details belong in `PROGRESS.md`, not
here — this file should only ever describe what's *not* done yet, plus
enough design context that whoever picks an item up next doesn't have
to re-derive decisions already made.

Last updated: 2026-11-27. Physical units Phase 2 (expanding
`UNIT_TABLE` to the full practical unit list, extending
`NAMED_DERIVED_SYMBOLS`, and the `README.md`/F1-overlay documentation
deferred from Phase 1) shipped this session — see `PROGRESS.md`. No
other open items remain.

## Open

Nothing queued right now.

## Known rough edges (not queued, revisit only if real use asks for them)

- A `for`'s collection field only accepts a reference to an
  already-defined range variable (`i:=1..3` then `for x in i`) — an
  inline range literal typed directly into the field (`for x in
  1..10`) fails with an unhelpful "trailing input" error, since
  `evaluateLatex()` has no range-literal handling at all (only
  `collectDefinitions()`'s `parseRangeDefinition()` does). Pre-existing,
  found while verifying a prior session's nesting-cap lift.
- A structure that's been typed into at all still can't be one-shot
  deleted via Backspace — intentional safety scope (see `PROGRESS.md`),
  not a gap.

