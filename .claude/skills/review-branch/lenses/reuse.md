LENS: reuse

Scope: new code that duplicates, bypasses, or weakens something the engine already provides. Before reporting R1 or R2 you must have run `rtk graphify query` or an LSP `workspaceSymbol`/`findReferences` lookup that shows the existing facility; name it in the sentence.

Checklist (cite the id):
- R1 Hand-rolled algorithm: a loop, lambda, or local helper that re-implements an `Engine.Common` or `Algo` function object (`next`, `clamp`, `transform`, `sort`, `find`, `contains`, `accumulate`, ...), or a std algorithm the engine already wraps. The fix is the shared helper, never an inlined copy, even when the inline version is shorter.
- R2 Duplicate facility: a new type, trait, concept, or free function whose purpose an existing engine symbol already serves (containers, smart pointers via `make_uptr`/`make_sptr`/`make_smr`, `Opt<T>`, `Expected`, string views, lock types, guards).
- R3 Local generalization: a helper placed in one bundle that at least one other bundle already needs or would need on the same branch; it belongs in the lowest bundle both depend on.
- R4 Lost generality: a formerly generic signature narrowed to one call site; a template replaced by a copy per type; a `ref<const T>` parameter turned into a by-value copy.
- R5 Dead flexibility: options, flags, virtuals, or template parameters with exactly one user on the branch and no issue text asking for more.
- R6 Vendored or std replacement: a vendored container or std facility replaced by an in-house one without a clear common-case win named in the commit message.

Severity: R1 and R2 are major; R3 to R6 minor.
