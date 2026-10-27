---
name: capture-issue
description: Use when the user hits a bug, design flaw, idea or docs gap mid-change and wants it captured as a GitHub issue draft for Heliogrim/Heliogrim-Engine. Trigger - /capture-issue <one line>. Never posts; the user reviews and submits a prefilled "new issue" page.
---

# Capture Issue

Turn a stumbled-on problem into a prefilled GitHub "new issue" page. The user reviews and submits. NEVER create the issue yourself, and never invent content for fields you cannot know.

`gh` may not be on PATH: use `C:\Program Files\GitHub CLI\gh.exe` (PowerShell). Repo: `Heliogrim/Heliogrim-Engine`, project: `Heliogrim/1` ("Heliogrim Project").

## 1. Classify

| Kind | Issue type | Labels | Template | Target status |
|---|---|---|---|---|
| Bug / problem | `Bug` | `bug`, `module::X`, `arch::X`, `platform::X` | `1-bug_report.yml` | To triage |
| Concrete feature | `Feature` | `enhancement` (+`performance` if about speed), `module::X`, `arch::X`, `platform::X` | `2-feature_request.yml` | Backlog |
| Idea / design flaw needing discussion | `Proposal` | `enhancement` or `question`, `module::X`, `arch::X`, `platform::X` | `2-feature_request.yml` | To discuss |
| Docs gap | `Documentation` | `documentation`, `module::X` (no arch/platform) | `2-feature_request.yml` | Backlog |

`Task` type is unused. If the kind is unclear, ask once.

- `module::X`: infer from touched file paths (`src/Engine.Reflow` -> `module::Reflow`, ...). Only use labels from `gh label list`; ask if a path does not map.
- `arch::*` / `platform::*`: use `*` when nothing is specific.

## 2. Snapshot (bugs; features only what the user said)

- branch (`git branch --show-current`), touched files (`git status --short`)
- short log excerpt, ALWAYS piped through `rtk`
- `path:line` where the problem shows

## 3. Duplicate check

`gh issue list --repo Heliogrim/Heliogrim-Engine --state open --search "<keywords>"`. Show close matches to the user before drafting.

## 4. Draft + open

Fill template fields by `id` (bug: `actual expected repro platform version subsystem context`; feature: `reasoning expectation done scope layout considerations information`). Leave `done`/`scope` EMPTY for the user; put the snapshot in `context`/`information`. Build and open (URL-encode values):

`https://github.com/Heliogrim/Heliogrim-Engine/issues/new?template=<file>&title=<t>&labels=<a,b>&type=<Type>&projects=Heliogrim/1&<id>=<value>...`

Open it with `Start-Process`. If the `type` parameter is ignored by GitHub, tell the user to set it by hand.

## 5. Status after submit

When the user says it is submitted: find the issue (`gh issue list --author @me --limit 3`), read its project item status via `gh project item-list 1 --owner Heliogrim --format json`, and if it differs from the target status set it with `gh project item-edit` (status field id `PVTSSF_lADOCwDwf84A0I09zgp0Uy4`; options: To triage `f971fb55`, Backlog `f75ad846`, To discuss `4cccfa3e`; project id `PVT_kwDOCwDwf84A0I09`). Needs the `project` token scope: if it fails, tell the user to run `! "/c/Program Files/GitHub CLI/gh.exe" auth refresh -s project`. Do not set Priority.
