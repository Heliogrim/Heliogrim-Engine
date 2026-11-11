LENS: drift

Scope: does the range do what the issue says, no more and no less, and does it respect the bundle layering in ARCHITECTURE.md. The issue text (title, body, "Done when", "Covered now / left for later") is in the prompt; if it is absent, run D4 and D5 only and put `issue text absent` under NOT REVIEWED.

Checklist (cite the id):
- D1 Undershoot: a "Done when" item with no code or test on the range that satisfies it.
- D2 Overshoot: a change that no issue line asks for and that is not required to make an asked-for change compile or pass; name the "left for later" line it pre-empts when there is one.
- D3 Scope leak: a problem found mid-change that was fixed inline instead of becoming its own issue and its own atomic commit (one commit touching an unrelated bundle is the usual sign).
- D4 Layering: a new include, link dependency, or `using` that makes a lower bundle depend on a higher one, or crosses a boundary ARCHITECTURE.md names as forbidden. Verify with `rtk graphify path "<from bundle>" "<to bundle>"`.
- D5 Naming drift: a new symbol or file that breaks the bundle's existing naming (short forms where the bundle spells names out, a `.Structured.cpp`/`.Layout.cpp` split not followed, a definition placed in a TU other than its declaring header's).
- D6 Commit shape: a commit whose message scope does not match its files, a fix folded into a feature commit, a commit touching more than one concern.

Severity: D1 and D4 are blocker; D2, D3, D6 major; D5 minor.
