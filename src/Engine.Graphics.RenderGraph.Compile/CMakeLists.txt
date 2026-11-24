
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.RenderGraph.Compile
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Graphics.Scene ${PROJECT_NAME}::Engine.Accel.Lang ${PROJECT_NAME}::Engine.Graphics.RenderGraph ${PROJECT_NAME}::Engine.Accel.Compile.Module
)
# shortcut: CompileVisitor.cpp imports Engine.Accel.Compile, linked as Engine.Accel.Compile.Module because Engine.Accel.Compile transitively links Engine.Graphics.RenderGraph.Compile via Engine.Graphics.Core and Engine.Graphics.Render, drop when Engine.Accel.Compile stops linking Engine.Graphics.Core