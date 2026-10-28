---
name: adr
description: Use when the user says "record this decision" or "ADR this", chooses between significant alternatives, or asks why something is shaped the way it is. Drafts a numbered Architecture Decision Record for the separate Heliogrim docs repo. Adapted from the ECC architecture-decision-records skill (Nygard format).
---

# ADR

Capture an architectural decision as a short numbered ADR. ADRs live in the separate Heliogrim docs repo, NEVER in this engine repo. Brainstorm specs and plans stay in GitHub issues or PRs, not in files here.

## Where

- Docs repo checkout: `$env:HELIOGRIM_DOCS_PATH`; if unset, ask the user once for the path. If the repo does not exist yet, stop and tell the user.
- Files: `<docs>/docs/adr/NNNN-<slug>.md`, index: `<docs>/docs/adr/README.md` (table: ADR, Title, Status, Date).
- Only accepted decisions get a file. A decision still under discussion (`proposed`) stays in its issue or PR until accepted.

## Format (keep it to about a page; Context at most 10 lines)

```markdown
# ADR-NNNN: <Decision Title>

**Date**: YYYY-MM-DD
**Status**: accepted | deprecated | superseded by ADR-NNNN
**Source**: <link to the issue or PR holding the discussion>

## Context
<2-5 sentences: situation, constraints, forces>

## Decision
<1-3 sentences, present tense: "We use X">

## Alternatives Considered
### <Name>
- **Pros**: ...
- **Cons**: ...
- **Why not**: ...

## Consequences
### Positive
### Negative
### Risks
```

## Workflow

1. Identify the decision and gather context, alternatives and consequences from the conversation. Use only what was actually decided; mark nothing as decided that the user did not decide.
2. Number it by scanning existing ADRs in the docs repo and incrementing.
3. Show the draft in chat. Write the file and the index row ONLY after explicit approval; if declined, discard.
4. Do not commit or push the docs repo; leave that to the user.
5. Reading ("why did we choose X?"): scan the index, read the matching ADR's Context and Decision, and answer from them. If none exists, say so and offer to record one.

## Don't

- Record trivial decisions (naming, formatting).
- Omit rejected alternatives ("we just picked it" is not a rationale).
- Backfill without marking the original date.
- Rewrite an accepted ADR; supersede it with a new one and link both ways.
