# Architecture

Heliogrim: experimental engine (Vulkan) plus editor. Orientation only. For symbols use LSP; for relationships use `graphify query`.
Layering below is inferred from bundle names; treat it as a map, verify before relying on it.

## Repo layout
- `src/<Bundle>/` one CMake library per directory, sources globbed (`MakeLibraryProject.cmake`). Tests are sibling bundles `<Bundle>.Test`.
- `assets/` binaries and encoded assets (never read), except `assets/shader/**`.
- `vendor/<name>/` third-party: `compile/` (never read), `include/` (shallow), `bin/`, `CMakeLists.txt`.
- `cmake/` toolchain and project macros. `dist/<Profile>/<triplet>/` binaries and test exes. `docs/` minor notes.
- `graphify-out/` knowledge graph; `Architecture.md` (this file) is the cheaper first stop.

## Naming
- `Engine.*` runtime, `Editor.*` tooling on top of it, `Heliogrim` public engine interface and glue, `Heliogrim.Main` process entry.
- `Support.<Name>.<Platform>` platform-specific implementations.
- Sub-bundles use dots: `Engine.Graphics.Material`, `Engine.Accel.Compile`. A `.Test` suffix is always a test bundle.

## Engine bundles by domain
Foundation (no engine deps expected; most bundles use these)
- `Engine.Common` containers, `Managed/` smart pointers (`Rc`, `Arci`, atomic refcounts), `Opt`, math, ids.
- `Engine.Asserts`, `Engine.Logging`, `Engine.Config`, `Engine.Env`, `Engine.Platform`, `Engine.Crypto`.
- `Engine.Pedantic(.Resource)` ownership/resource guards. `Engine.Async` tasks and futures. `Engine.Event`.
- `Engine.Filesystem`, `Engine.IO.Dispatcher`, `Engine.Input(.Schedule)`.

Core and simulation
- `Engine.Core` (+ `.Game`, `.Schedule`): engine, universe, world bootstrap and scheduling.
- `Engine.ACS` (+ `.Schedule`): actor/component system; `Housekeeping.cpp`, actor archetypes in progress.
- `Engine.Level`, `Engine.Physics`, `Engine.Physics.Scene(.Model)`, `Engine.Physics.Schedule`: levels and physics.
- `Engine.Audio` audio. `Engine.Network` networking.

Assets
- `Engine.Assets` (+ `.Type`, `.System`): asset model, typed assets (e.g. geometry), `AssetRegistry`.

Graphics (Vulkan)
- `Engine.Driver.Vulkan` device layer. `Engine.Graphics` (`Graphics.cpp`) front, plus `.Geometry`, `.Glow`, `.IO`, `.Loader`, `.Event`.
- `Engine.Graphics.Scene(.Model)`, `Engine.Graphics.Schedule`: render-side scene mirror and frame scheduling.
- `Engine.Graphics.Render(.Command/.Graph.Scene/.Predefined/.Subpass)` and `Engine.Graphics.RenderGraph(.Compile)`: pass and render-graph execution.
- `Engine.Graphics.Material`: materials, prototypes, effect usage patterns, parameter storage.

Acceleration (shader/pipeline compilation, replaces older graphics pipeline code)
- `Engine.Accel.Lang` shader language/emitters (`GlslEmitter`). `Engine.Accel.Effect` effect and stage description.
- `Engine.Accel.Compile` effect compiler, profiles, specs, `VkModuleBuilder`. `Engine.Accel.Pipeline` pipeline/stage modules.
- `Engine.Accel.Pass`, `.Command`, `.Storage`, `.Loader`: passes, commands, caches, loading.
- Shaders live in `assets/shader/**` (readable).

## Editor bundles
- `Editor.Core` (`EditorEngine`), `Editor.Boot` (asset and serialization init), `Editor.Main` entry.
- `Editor.UI` (widgets, viewport), `.UI.Panel` (scene hierarchy etc.), `.UI.View`, `.UI.Theme`, `.UI.Service`, `.UI.Rx` (reactive state), `.UI.Init`, `.UI.Main`.
- `Editor.Action` (+ `.Assets.Actions`, `.Scene.Actions`): undoable/import actions such as `AutoImportAction`.
- `Editor.Assets.Default` built-in assets (fonts). `Editor.Scene`, `Editor.Render`, `Editor.Graphics.*` editor-side render and IO.

## Build graph
- Targets: `Heliogrim-Editor` (everything), `Heliogrim-<Bundle>.Test`, `Heliogrim-AllTests`. Unity build is closed/off; do not assume it.
- Vendored libs build through `vendor/<name>/CMakeLists.txt`; versions bumped on `build/*` branches (e.g. ankerl 5.3.3).

## Runtime flow (high level)
1. `Heliogrim.Main` (or `Editor.Main`) creates the engine; `Editor.Boot` registers assets and serializers.
2. `Engine.Core` owns universe/levels; `Engine.ACS` ticks actors on the core schedules.
3. Game state is mirrored to `Engine.Graphics.Scene`; `Engine.Graphics.Render*` builds a render graph per frame and records via `Engine.Driver.Vulkan`.
4. Materials resolve through `Engine.Graphics.Material` to effects compiled by `Engine.Accel.Compile` into Vulkan modules/pipelines.
5. Editor panels (`Editor.UI*`) drive the engine through `Editor.Action` and render into `Editor.UI` viewports.

## Conventions worth knowing
- `Opt`: `Some(lvalue)` is a reference, `SomeVal` is a value. Handled types spell `const` explicitly, no propagation.
- Commits: conventional, atomic, scoped by bundle (`fix(accel-compile): ...`); linear history, no AI trailers.
- Do not replace vendored containers without a clear common-case win.
- Design drafts and exploration notes live in the separate Pages docs repo, never on `master`.
