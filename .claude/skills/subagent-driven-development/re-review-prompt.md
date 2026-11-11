# Scoped Re-Review Prompt Template

Dispatch after a fix round with the `cpp-reviewer` agent preset
(`.claude/agents/cpp-reviewer.md`). The re-reviewer verifies the findings
were addressed and checks the fix diff for new breakage. It is not a fresh
review — the full review already happened.

**Purpose:** Verify each finding from the previous review was addressed, and
that the fix itself broke nothing.

```
Agent:
  subagent_type: cpp-reviewer
  description: "Re-review Task N fix round R"
  model: [MODEL — per SKILL.md Model Selection; alias only (haiku|sonnet|opus|fable),
         overrides the preset default]
  prompt: |
    You are re-reviewing one task's fix round. A previous review produced
    findings; an implementer attempted to fix them. Verdict each finding and
    inspect the fix diff — nothing else.

    ## The Task

    Task brief: [BRIEF_FILE]

    ## The Findings Under Verification

    [FINDINGS]

    ## The Fix

    Implementer's report (fix reports appended at the end): [REPORT_FILE]

    **Fix base:** [FIX_BASE_SHA] (the head the previous review saw)
    **Head:** [HEAD_SHA]  **Diff file:** [DIFF_FILE]

    Read the diff file once — fix commits, stat summary, and fix diff with
    context. Do not re-run git commands. If the diff file is missing:
    `git diff --stat [FIX_BASE_SHA]..[HEAD_SHA]` and
    `git diff [FIX_BASE_SHA]..[HEAD_SHA]`.

    Read-only: do not mutate the working tree, index, HEAD, or branch state.

    ## You Do Not Dispatch Subagents

    Do all of this review yourself. Never spawn a subagent for part of the
    diff or another reviewer for a second opinion: every review seat is
    already provided, a spawned one duplicates it at full cost, and its
    verdict counts for nothing. If the diff is too large for one pass, review
    it in passes yourself and say so.

    ## Scope

    Your scope is the findings list and the fix diff. Verdict every finding.
    Inspect the fix diff for new problems the fix introduced. Do NOT
    re-review code the fix did not touch: an issue entirely outside the fix
    diff goes under Out-of-Scope Observations — it does not block this task
    or extend the loop. A broad whole-branch review happens after all tasks
    are complete. Need a call site to judge a fix? One LSP lookup, not a
    crawl.

    ## Tests

    The implementer re-ran the tests covering the amended code and appended
    the results to the report file. Treat the report as unverified claims:
    confirm the fix report names the covering tests and shows their output,
    and verify the claims against the diff. Do not re-run the suite to
    confirm the report. Run a test only when the code raises a specific doubt
    no existing run answers — then one focused test (through rtk), never a
    whole bundle.

    ## Output Format

    This format overrides the preset's default return format. Your final
    message is the report itself: begin directly with the first finding's
    verdict. Every line is a verdict, a finding with file:line, or a check
    you ran — no preamble, no process narration.

    ### Finding Verdicts

    For each finding in The Findings Under Verification, in order:
    - **[finding one-liner]** — ADDRESSED | NOT ADDRESSED, with file:line
      evidence. "Attempted" is not addressed: the specific defect must no
      longer exist.

    ### New Breakage in the Fix Diff

    Anything the fix itself broke or introduced, with severity
    (Critical/Important/Minor) and file:line. "None" if clean.

    ### Out-of-Scope Observations

    Issues entirely outside the fix diff. Non-blocking; the controller
    ledgers these for the final review. "None" if none.

    ### Verdict

    **Fix round:** [All findings addressed, no new Critical/Important
    breakage | Findings remain open] — list the open ones.
```

**Placeholders:**
- `[MODEL]` — reviewer model per SKILL.md Model Selection (alias only); scoped
  re-reviews of small fix diffs take a cheap-to-mid tier
- `[BRIEF_FILE]` — the task brief file (same file the implementer worked from)
- `[FINDINGS]` — the Critical/Important findings and spec gaps from the
  previous review, copied verbatim, one per bullet
- `[REPORT_FILE]` — the implementer's report file (fix reports appended)
- `[FIX_BASE_SHA]` — the head the previous review saw
- `[HEAD_SHA]` — current commit
- `[DIFF_FILE]` — the path `bash scripts/review-package PLAN_FILE FIX_BASE HEAD` printed

**Re-reviewer returns:** per-finding verdicts (ADDRESSED / NOT ADDRESSED),
new breakage in the fix diff, out-of-scope observations, and a round verdict.
