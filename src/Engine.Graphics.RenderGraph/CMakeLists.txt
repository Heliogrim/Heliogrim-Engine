
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.RenderGraph
        MODULE
        MODULE_EXCLUDE
        # Unused and stale: includes a missing ../Component.hpp and names a Component from another namespace.
        Engine.Graphics.RenderGraph/Component/CompileComponent.hpp
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.Resource ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Graphics.Scene ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Graphics.Types ${PROJECT_NAME}::Engine.Storage.Registry ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Accel.Lang ${PROJECT_NAME}::Engine.Accel.Effect ${PROJECT_NAME}::Engine.Graphics.RenderGraph.Compile.Module ${PROJECT_NAME}::Engine.Accel.Pass.Module ${PROJECT_NAME}::Engine.Accel.Pipeline.Module ${PROJECT_NAME}::Engine.Accel.Command.Module
)
# shortcut: Engine.Graphics.RenderGraph/test.cpp imports Engine.Accel.Pipeline (StageModule) and reaches Engine.Accel.Command via Engine.Graphics.Render.Command/Commands/Structs.hpp, linked as .Module targets because Engine.Accel.Pipeline (and Engine.Accel.Command through it) transitively links Engine.Graphics.RenderGraph, drop when test.cpp moves into a test project
# shortcut: Engine.Graphics.RenderGraph/test.cpp imports Engine.Graphics.RenderGraph.Compile and reaches Engine.Accel.Pass via Engine.Graphics.Render.Command/Commands/Structs.hpp, drop when test.cpp moves into a test project