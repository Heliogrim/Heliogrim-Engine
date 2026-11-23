
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.Scene.Model
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Graphics.Components ${PROJECT_NAME}::Engine.Render.Scene
        ${PROJECT_NAME}::Engine.Pedantic.Resource ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type
)