# TODO

Running backlog of requested features, tracked by the manager session
so a new session can pick up immediately without re-reading the whole
conversation history. Update status here as items move; once something
is actually done and verified, its details belong in `PROGRESS.md`, not
here — this file should only ever describe what's *not* done yet, plus
enough design context that whoever picks an item up next doesn't have
to re-derive decisions already made.

Last updated: 2026-11-26. Backspace-deleting a whole pristine
`for`/`while`/`if` structure, lifting program blocks' v1 nesting cap
(any number of `for`/`while`/`if`, any depth), general matrix indexing
(`M_{i,j}`/`M_i`, `row()`/`col()`), an in-app `F1` help overlay, and
physical units Phase 1 (core quantity type + a 10-unit proof-of-concept
set) all shipped this session — see `PROGRESS.md`.

## Open

- **Physical units, Phase 2** — expand `UNIT_TABLE`
  (`src/notebook.html`, next to `freshBuiltinVars()`) with the full
  practical unit list; Phase 1 deliberately shipped only 10 units
  (`m`/`s`/`A`/`K`/`N`/`kg`/`mol`/`cd`/`Pa`/`Hz`) to validate the core
  architecture first. This is now purely a data-entry task — the
  arithmetic/display machinery (`quantityBroadcast()`,
  `appendQuantityResult()`, `collapseIfDimensionless()`, the
  `dimsLabel()` compound-label fallback) needs no further changes,
  confirmed by Phase 1's own design. For each new unit, decide only:
  symbol, its `dims` vector, its seed value (canonical-SI value of one
  of that unit — e.g. a future `ft` entry seeds `0.3048`), and whether
  it's single-letter (shadowable, goes in `freshBuiltinVars()`) or
  multi-letter (permanently reserved, goes in the `MULTI_LETTER_UNITS`
  literal-match list in `parseAtom`, ordered longest-symbol-first so a
  new entry sharing a prefix with an existing one doesn't need
  restructuring — the list is already written this way for exactly
  this reason). Candidates worth including (not an exhaustive
  mandate — use judgment on what's actually useful): `ft`/`in`/`mi`
  (length), `lb`/`oz`/`g` (mass — `g` gram, distinct from the existing
  `kg` base), `hr`/`min` (time), `cm`/`mm`/`km` (metric length
  prefixes — SI-prefix COMBINATORICS across every unit is still out of
  scope, these are individually hand-enumerated), `J`/`W`/`V`/`Ω`/`F`/
  `Wb`/`T`/`H`/`C` (more named SI derived units, extending
  `NAMED_DERIVED_SYMBOLS`), `gal` (volume, exercises a `length:3` dims
  vector — nothing new needed, just confirms the mechanism generalizes).
  No Celsius/Fahrenheit (additive-offset units need genuinely different
  math than this feature's pure multiplicative scaling — a real,
  separate design question, not scoped here).
  - **Also update `README.md`/the `F1` help overlay** once Phase 2's
    unit list is in: deliberately NOT done for Phase 1 — a 10-unit
    proof-of-concept set isn't yet the feature's real end-user shape,
    and documenting it now would just mean rewriting the same section
    again immediately after Phase 2. Not an oversight; do it once,
    after Phase 2.

## Known rough edges (not queued, revisit only if real use asks for them)

## Known rough edges (not queued, revisit only if real use asks for them)

- A `for`'s collection field only accepts a reference to an
  already-defined range variable (`i:=1..3` then `for x in i`) — an
  inline range literal typed directly into the field (`for x in
  1..10`) fails with an unhelpful "trailing input" error, since
  `evaluateLatex()` has no range-literal handling at all (only
  `collectDefinitions()`'s `parseRangeDefinition()` does). Found while
  verifying the nesting-cap lift above; pre-existing, unrelated to
  nesting.
- A structure that's been typed into at all still can't be one-shot
  deleted via Backspace — intentional safety scope (see `PROGRESS.md`),
  not a gap.

