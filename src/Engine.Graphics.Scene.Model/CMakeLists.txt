
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.Scene.Model
        MODULE
        MODULE_EXCLUDE
        # Unused and stale: includes a missing Engine.Assets/Types/LandscapeGeometry.hpp and overrides a scene::Scene interface that no longer exists.
        Engine.Graphics.Scene.Model/Scene/LandscapeGeometryModel.hpp
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.Resource ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Graphics.Components ${PROJECT_NAME}::Engine.Render.Scene
        ${PROJECT_NAME}::Engine.Pedantic.Resource ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Accel.Lang ${PROJECT_NAME}::Engine.Graphics.RenderGraph ${PROJECT_NAME}::Engine.Graphics.Loader
)