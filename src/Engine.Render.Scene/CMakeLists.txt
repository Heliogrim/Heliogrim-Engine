
# Setup
make_library_project(
        LIB_NAME Engine.Render.Scene
        MODULE
        LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Scene ${PROJECT_NAME}::Engine.Graphics.Types ${PROJECT_NAME}::Engine.Graphics.Core.Module
)
# shortcut: RenderSceneSystem.cpp imports Engine.Graphics.Core, linked as Engine.Graphics.Core.Module because Engine.Graphics.Core links Engine.Render.Scene; it calls Graphics::get()->getCurrentDevice(), drop when the device is passed in