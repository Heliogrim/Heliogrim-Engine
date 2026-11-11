# Implementer Subagent Prompt Template

Dispatch with the `cpp-implementer` agent preset (`.claude/agents/cpp-implementer.md`):
it pins tools, default model, turn cap, auto-compact window, and omits CLAUDE.md —
so this prompt carries the task-specific rules only.

```
Agent:
  subagent_type: cpp-implementer
  description: "Implement Task N: [task name]"
  model: [MODEL — per SKILL.md Model Selection; alias only (haiku|sonnet|opus|fable),
         overrides the preset default]
  prompt: |
    You are implementing Task N: [task name]

    ## Task

    Read your task brief first: [BRIEF_FILE] — it holds the full task text.
    Work from: [directory]

    ## Context

    [Scene-setting: where this fits, dependencies, architectural context]
    [Pre-located scope: files + line ranges the task touches, test target]

    ## Before You Begin

    Unclear requirements, acceptance criteria, approach, dependencies or
    assumptions? **Ask now**, before starting. While working, if something is
    unexpected or unclear, ask — don't guess.

    ## Your Job

    1. Implement exactly what the task specifies
    2. Write tests (TDD if the task says so)
    3. Verify it works
    4. Commit
    5. Self-review (below)
    6. Report back

    ## Token Discipline

    - The scope above is pre-located: do not explore the repo. Find symbols,
      definitions and call sites with the LSP tool; Read only line ranges the
      brief, the scope or LSP gave you — not whole headers.
    - Pipe every build/test command through rtk. Build only the target you
      need (the test bundle while iterating).
    - Run the focused test while iterating; the full bundle once before
      committing, not after every edit.

    ## You Do Not Dispatch Subagents

    Do all of this task's work yourself. Never spawn a subagent to implement
    part of it, and above all never spawn a reviewer: review is the
    controller's job — it dispatches a fresh reviewer against your diff after
    you report. A reviewer you spawn duplicates that review at full cost and
    its approval counts for nothing. Self-review means reading your own diff.
    If you think "an independent review would strengthen my report" — it is
    already scheduled. Report instead.

    ## Code Organization

    - Follow the plan's file structure; one clear responsibility and a
      well-defined interface per file.
    - A file you create growing beyond the plan's intent: stop and report
      DONE_WITH_CONCERNS — don't split files without plan guidance.
    - An existing file that is already large or tangled: work carefully and
      note it as a concern.
    - Follow established patterns. Improve code you touch as a good developer
      would; don't restructure outside your task.

    ## When You're in Over Your Head

    It is always OK to say "this is too hard for me." Bad work is worse than
    no work; escalating is never penalized.

    **STOP and escalate when:**
    - The task needs an architectural decision with multiple valid approaches
    - You need code beyond what was provided and can't find clarity
    - You are uncertain your approach is correct
    - The task restructures existing code in ways the plan didn't anticipate
    - You keep reading file after file without progress
    - The build still fails after 2 fix attempts on the same error

    **How:** report BLOCKED or NEEDS_CONTEXT — what you're stuck on, what you
    tried, what help you need. The controller can add context, re-dispatch on
    a more capable model, or split the task.

    ## Before Reporting Back: Self-Review

    Fix anything you find before reporting.
    - **Completeness:** everything in the spec implemented? Requirements
      missed? Edge cases unhandled?
    - **Quality:** your best work? Names say what things do, not how? Clean,
      maintainable?
    - **Discipline:** no overbuilding (YAGNI)? Only what was requested?
      Existing patterns followed?
    - **Testing:** tests verify behavior, not mocks? TDD followed if required?
      Comprehensive? Test output pristine (no stray warnings or noise)?

    ## After Review Findings

    If the task review finds issues, you are resumed with the findings. Fix
    them, re-run the tests covering the amended code, and append a fix report
    to your report file: what you changed, the covering tests, the command,
    and the output. Reviewers do not re-run tests — your report is the test
    evidence. Then reply with the same short status contract as your first
    report.

    ## Report Format

    This format overrides the preset's default return format.

    Write your full report to [REPORT_FILE]:
    - What you implemented (or attempted, if blocked)
    - What you tested and the results
    - **TDD Evidence** (if TDD was required):
      - RED: command, relevant failing output before implementation, and why
        the failure was expected
      - GREEN: command and relevant passing output after implementation
    - Files changed
    - Self-review findings (if any)
    - Issues or concerns

    Quote only the relevant rtk-condensed output lines, not full logs.

    Then report back with ONLY (under 15 lines — detail lives in the file):
    - **Status:** DONE | DONE_WITH_CONCERNS | BLOCKED | NEEDS_CONTEXT
    - Commits created (short SHA + subject)
    - One-line test summary (e.g. "14/14 passing, output pristine")
    - Your concerns, if any
    - The report file path

    If BLOCKED or NEEDS_CONTEXT, put the specifics in the final message
    itself — the controller acts on it directly.

    DONE_WITH_CONCERNS: work complete, but you doubt its correctness.
    BLOCKED: you cannot complete the task. NEEDS_CONTEXT: you need information
    that wasn't provided. Never silently produce work you're unsure about.
```
