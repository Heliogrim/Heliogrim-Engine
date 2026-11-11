---
name: review-branch
description: Use when a feature branch or commit range is ready for owner review, before a PR is opened or marked ready, or when the owner says "review the branch", "run the lenses", or /review-branch <range> [issue#]. Produces one merged, ranked findings list from parallel single-focus reviewers.
---

# Review Branch

One ranked list instead of six transcripts. Deterministic checks run first and cost nothing; LLM lenses run in parallel on the same range; the merge step dedups and ranks; the owner reads one list.

**REQUIRED SUB-SKILL:** dispatching-parallel-agents for the fan-out.

## Inputs

- `<range>`: `master..HEAD` by default; for a stacked branch use `<parent-branch>..HEAD`.
- `[issue#]`: the GitHub issue that is the spec. Without it the drift lens runs in layering-only mode and says so.

## 1. Deterministic gate (no LLM)

Run all of these, through `rtk`, and stop if the build or tests fail; a lens over broken code is wasted spend.

```powershell
rtk git diff --name-only <range> -- 'src/*.cpp' 'src/*.hpp' > $env:TEMP\changed.txt
rtk clang-format --dry-run (Get-Content $env:TEMP\changed.txt)   # fail only on include order/grouping or a missing final newline; the rest is ReSharper residue, see STYLE.md
rtk clang-tidy -p .\cmake-build-debug-visual-studio-clang (Get-Content $env:TEMP\changed.txt | Where-Object { $_ -like '*.cpp' })
rtk cmake --build .\cmake-build-debug-visual-studio-clang --target Heliogrim-AllTests -j 7
rtk .\dist\Debug\amd64-windows\Heliogrim-<Bundle>.Test.exe --gtest_color=no   # each bundle the range touches
```

The gate owns: include sorting and grouping, column limit, trailing newline, brace placement, everything `.clang-tidy` lists. Lenses do not re-report these. STYLE.md wins over `.clang-format` where they differ; the formatter's known residue (lambda arguments, `requires` placement, breaks under the limit) is never a finding.

## 2. Lenses (parallel)

Dispatch one `lens-reviewer` agent per lens in ONE response. Each prompt is: the lens file contents from `lenses/`, the range, the issue text (drift lens only), the full text of `STYLE.md` (style lens only), and nothing else. The agent has no session history.

Shard large ranges instead of raising `maxTurns`: when the range touches more than ~80 added/modified source files, give each lens one agent per bundle group of at most ~80 files (split by `src/<Bundle>` prefix, keep a bundle and its `.Test` together), all in the same response. Pass the shard's file list in the prompt. A single agent over a large range runs out of turns or auto-compacts and silently drops findings; shards cost the same tokens and run in parallel.

| Lens | File | Model |
|------|------|-------|
| Style and layout | `lenses/style.md` | sonnet |
| Reuse and sustainability | `lenses/reuse.md` | opus |
| Drift (spec and layering) | `lenses/drift.md` | opus |
| Security (trust boundaries) | `lenses/security.md` | opus |
| Tests (coverage and efficiency) | `lenses/tests.md` | opus |
| Correctness and lifetime | existing `cpp-reviewer` agent, no lens file | opus |

If the range touches `src/Engine.Async/**`, `src/Engine.Scheduler/**`, or any file containing `std::atomic`, also open the verifying-concurrency skill and run its checklist; a lens cannot replace it.

## 3. Merge

Concatenate the lens outputs, then:
1. Group by `file:line`; a line reported by two lenses keeps the higher severity and lists both lens names.
2. Drop findings whose checklist id the lens file does not contain (hallucinated items).
3. Order: blocker, major, minor; within a severity, by file path.
4. Append every `NOT REVIEWED` entry under one heading so the owner knows the holes.

Write the merged list to `docs/superpowers/reviews/<date>-<branch-slug>.md` (not committed; `docs/superpowers/**` is a local scratch tree) and show it in the reply.

## 4. Fix loop

The owner triages the list. Each accepted finding becomes one task for `cpp-implementer` with the exact file and line range; its result is a `fixup!` commit onto the commit that introduced the line (`git log -L` or `git blame` on the range). Re-run only the lenses whose findings were accepted, on `<old-HEAD>..HEAD`.

## Common mistakes

- Running lenses before the gate passes: findings on code that does not compile are noise.
- Giving a lens the owner's goals or session history: it then reviews intent instead of the diff.
- Letting a lens report outside its checklist: that is the cause of overlapping, contradictory findings. The agent drops those; the merge step drops any that slip through.
- Fixing in the review session instead of via `cpp-implementer`: the reviewer context is gone and the fix is unreviewed.
