
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.Schedule
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Graphics.Scene ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Scene.Schedule ${PROJECT_NAME}::Engine.Graphics.Types ${PROJECT_NAME}::Engine.Graphics.RenderGraph ${PROJECT_NAME}::Engine.Graphics.Render ${PROJECT_NAME}::Engine.Graphics.Core.Module
)
# shortcut: Engine.Graphics.Schedule .cpp files reach the graphics front via getEngine()->getGraphics(), drop when the service-locator design lands
# shortcut: RenderTickStage.cpp imports Engine.Graphics.Core, linked as Engine.Graphics.Core.Module because Engine.Graphics.Core links Engine.Graphics.Schedule, drop when the service-locator design lands