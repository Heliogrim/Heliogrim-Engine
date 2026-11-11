# Heliogrim (C++23 engine + editor, Windows, clang-cl, Ninja)

Orientation: read `ARCHITECTURE.md` first. Layout, bundle names and layering live there, not here.

## Workflow
- The GitHub issue is the spec: read it (incl. "Done when", "Covered now / left for later") before starting.
- Branches are `<type>/<slug>`. No issue refs in branch names or commits; link issues from the PR description only.
- A problem found mid-change becomes its own human-verified issue; its fix is an atomic commit that can be split into its own PR by rebase.
- Before a PR: `/review-branch <range> [issue#]` (deterministic gate, then parallel lens reviewers, one merged list). Concurrency primitives additionally follow the `verifying-concurrency` skill (TLA+ model as `<Primitive>.tla` + `.cfg` beside the source, Relacy, stress test).

## Code
- Language level is C++23 (`-std=c++23`). No C++ modules in use (no `.ixx/.cppm` files). Do not use C++26-only features.
- Edit `.cpp` only, unless a new declaration needs an `.hpp` change.
- Follow `STYLE.md` (includes, `ptr<T>`/`ref<const T>`, class layout, `X.Structured.cpp` / `X.Layout.{hpp,cpp}` split).
- Bundles are globbed per directory (`src/<Bundle>/**`); a new file needs a CMake reconfigure.

## Finding things (in this order)
1. Symbol, type, overload, reference: `LSP` (clangd is authoritative; the graph misses templates and concepts).
2. Which bundle or file owns X, how do bundles relate: `ARCHITECTURE.md`, then `graphify query|path|explain`.
3. `graphify-out/GRAPH_REPORT.md` and `wiki/index.md`: only for a broad architecture review.
4. Never grep whole directories for a symbol. After code changes run `graphify update .` (AST-only, free).
   A large `graph.json` diff in a PR is expected; do not flag it.

## Never read
- `assets/**` except `assets/shader/**` (binary or encoded).
- `vendor/<name>/compile/**` (third-party source). `vendor/<name>/include/**`: shallow reads only, when required.

## Build and test (always prefix with `rtk`; this includes logs, build output and errors)
Build dir: `cmake-build-debug-visual-studio-clang` (alt. MSVC cl: `cmake-build-debug`).
```
rtk cmake --build .\cmake-build-debug-visual-studio-clang --target <T> -j 7
```
Targets `<T>`: `Heliogrim-Editor` (editor+engine), `Heliogrim-<Bundle>.Test` (one bundle), `Heliogrim-AllTests`.
Run a test: `rtk .\dist\Debug\amd64-windows\Heliogrim-<Bundle>.Test.exe --gtest_color=no` (add `--gtest_filter=` while iterating).

Configure, only if the build dir is missing or CMake files changed:
```powershell
cmake.exe -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=ninja "-DCMAKE_C_COMPILER=clang-cl.exe" "-DCMAKE_CXX_COMPILER=clang-cl.exe" "-DCMAKE_RC_COMPILER=llvm-rc.exe" "-DCMAKE_RC_FLAGS=/C 1252" -G Ninja -S X:\Dev\C++\Heliogrim-CLion -B X:\Dev\C++\Heliogrim-CLion\cmake-build-debug-visual-studio-clang
```
