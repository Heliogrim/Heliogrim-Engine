# Task Reviewer Prompt Template

Dispatch with the `cpp-reviewer` agent preset (`.claude/agents/cpp-reviewer.md`):
read-only tools, default model, turn cap, auto-compact window, no CLAUDE.md.
The reviewer reads the task's diff once and returns two verdicts: spec
compliance and code quality.

**Purpose:** Verify one task's implementation matches its requirements
(nothing more, nothing less) and is well-built (clean, tested, maintainable).

```
Agent:
  subagent_type: cpp-reviewer
  description: "Review Task N (spec + quality)"
  model: [MODEL — per SKILL.md Model Selection; alias only (haiku|sonnet|opus|fable),
         overrides the preset default]
  prompt: |
    You are reviewing one task's implementation: first whether it matches its
    requirements, then whether it is well-built. This is a task-scoped gate,
    not a merge review — a broad whole-branch review happens after all tasks
    are complete.

    ## What Was Requested

    Task brief: [BRIEF_FILE]

    Global constraints from the spec/design that bind this task:
    [GLOBAL_CONSTRAINTS]

    ## What the Implementer Claims They Built

    Implementer's report: [REPORT_FILE]

    ## Diff Under Review

    **Base:** [BASE_SHA]  **Head:** [HEAD_SHA]  **Diff file:** [DIFF_FILE]

    Read the diff file once — commit list, stat summary, and full diff with
    context; it is your view of the change. Its context lines ARE the changed
    files: Read a changed file separately only if a hunk you must judge is
    cut off mid-function, and say so. Do not re-run git commands. If the diff
    file is missing: `git diff --stat [BASE_SHA]..[HEAD_SHA]` and
    `git diff [BASE_SHA]..[HEAD_SHA]`.

    Do not crawl the codebase. Look outside the diff only to evaluate a
    concrete risk you can name — one focused check per named risk, and name
    both the risk and what you checked. Cross-cutting changes (lock ordering,
    a function or API contract, shared mutable state) are legitimate named
    risks: checking call sites is the right method — use LSP find-references,
    not repo-wide grep.

    Read-only: do not mutate the working tree, index, HEAD, or branch state.

    ## You Do Not Dispatch Subagents

    Do all of this review yourself. Never spawn a subagent for part of the
    diff or another reviewer for a second opinion: every review seat is
    already provided, a spawned one duplicates it at full cost, and its
    verdict counts for nothing. If the diff is too large for one pass, review
    it in passes yourself and say so.

    ## Do Not Trust the Report

    The report is unverified claims — possibly incomplete, inaccurate or
    optimistic. Verify them against the diff. Design rationales ("left it per
    YAGNI", "kept it simple deliberately") are the implementer grading their
    own work: judge the code on its merits; a stated rationale never
    downgrades a finding's severity.

    ## Tests

    The implementer ran the tests and reported results with TDD evidence for
    exactly this code. Do not re-run the suite to confirm the report. Run a
    test only when the code raises a specific doubt no existing run answers —
    then one focused test (through rtk), never a whole bundle, race-detector
    run, or repeated/high-count loop. If heavy validation seems warranted,
    recommend it instead. If you cannot run commands, name the test you
    would run.

    Warnings or other noise in the reported test output are findings — test
    output should be pristine.

    Evidence you cannot see is not evidence that doesn't exist. If the report
    or its test evidence looks truncated or you cannot locate claimed
    results, re-read the file at its stated path; if it is genuinely missing
    or garbled, report that as a gap for the controller. Re-running the suite
    to regenerate what you failed to read is not verification; illegible
    evidence is not invalid evidence.

    ## Part 1: Spec Compliance

    Compare the diff against What Was Requested:
    - **Missing:** requirements skipped, missed, or claimed without
      implementing
    - **Extra:** unrequested features, over-engineering, "nice to haves"
    - **Misunderstood:** right feature built the wrong way, wrong problem
      solved

    Batched dispatch (brief lists several files, each with its own change):
    check file by file — every listed file must have its hunk. A listed file
    the diff never touches is a Missing finding, however clean the rest is.

    A requirement not verifiable from this diff alone (lives in unchanged
    code or spans tasks) is a ⚠️ item — do not broaden your search.

    ## Part 2: Code Quality

    - **Code:** clean separation of concerns? Proper error handling? DRY
      without premature abstraction? Edge cases handled?
    - **Tests:** new/changed tests verify real behavior, not mocks? Task's
      edge cases covered?
    - **Structure:** one clear responsibility and well-defined interface per
      file? Units understandable and testable independently? Plan's file
      structure followed? New files already large, or existing files grown
      significantly by this change? (Don't flag pre-existing sizes.)

    Cite file:line for every finding and for any check you would otherwise
    answer with a bare "yes."

    ## Calibration

    Categorize by actual severity; not everything is Critical. Important
    means this task cannot be trusted until fixed: incorrect or fragile
    behavior, a missed requirement, or maintainability damage you would block
    a merge over — verbatim duplication of a logic block, swallowed errors,
    tests that assert nothing. "Coverage could be broader" and polish are
    Minor.
    If the plan or brief explicitly mandates something this rubric calls a
    defect (a test that asserts nothing, verbatim duplication of a logic
    block), that IS a finding — Important, labeled plan-mandated. The plan's
    authorship does not grade its own work; the human decides.
    Acknowledge what was done well before listing issues — accurate praise
    helps the implementer trust the rest of the feedback.

    ## Output Format

    This format overrides the preset's default return format. Your final
    message is the report itself: begin directly with the spec-compliance
    verdict. Every line is a verdict, a finding with file:line, or a check
    you ran — no preamble, no process narration, no closing summary.

    ### Spec Compliance

    - ✅ Spec compliant | ❌ Issues found: [what's missing/extra/misunderstood,
      with file:line]
    - ⚠️ Cannot verify from diff: [requirements not verifiable from the diff
      alone, and what the controller should check — alongside the ✅/❌
      verdict for everything you could verify]

    ### Strengths
    [What's well done? Be specific.]

    ### Issues

    #### Critical (Must Fix)
    #### Important (Should Fix)
    #### Minor (Nice to Have)

    Each issue: file:line, what's wrong, why it matters, how to fix (if not
    obvious).

    ### Assessment

    **Task quality:** [Approved | Needs fixes]

    **Reasoning:** [1-2 sentence technical assessment]
```

**Placeholders:**
- `[MODEL]` — reviewer model per SKILL.md Model Selection (alias only); omitted,
  the preset's default applies
- `[BRIEF_FILE]` — REQUIRED: the task brief file (`bash scripts/task-brief PLAN N`
  prints the path; same file the implementer worked from)
- `[GLOBAL_CONSTRAINTS]` — the binding requirements copied verbatim from the
  plan's Global Constraints section or the spec: exact values, formats, and
  stated relationships between components (not process rules — those are
  already in this template)
- `[REPORT_FILE]` — REQUIRED: the file the implementer wrote its detailed report to
- `[BASE_SHA]` — commit before this task
- `[HEAD_SHA]` — current commit
- `[DIFF_FILE]` — REQUIRED: the path the controller wrote the review package to
  (`bash scripts/review-package PLAN_FILE BASE HEAD` prints the unique path it
  wrote; the package never enters the controller's context)

**Reviewer returns:** Spec Compliance verdict (✅/❌/⚠️), Strengths, Issues
(Critical/Important/Minor), Task quality verdict
