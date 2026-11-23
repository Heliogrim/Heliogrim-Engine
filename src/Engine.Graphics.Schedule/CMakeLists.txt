
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.Schedule
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Graphics.Scene ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Scene.Schedule ${PROJECT_NAME}::Engine.Graphics.Types
)
# shortcut: Engine.Graphics.Schedule .cpp files reach the graphics front via getEngine()->getGraphics(), drop when the service-locator design lands
# shortcut: RenderTickStage.cpp uses Engine.Graphics.Render (RenderTarget), not linked because the link closes a cycle, drop when the service-locator design lands