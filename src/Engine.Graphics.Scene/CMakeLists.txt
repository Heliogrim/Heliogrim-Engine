
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.Scene
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Input ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Resource.Archive
        ${PROJECT_NAME}::Engine.Platform ${PROJECT_NAME}::Engine.Scene ${PROJECT_NAME}::Engine.Render.Scene ${PROJECT_NAME}::Engine.Render.Scene.Model.Module
)
# shortcut: RenderSceneManager.Inject.cpp uses Engine.Graphics.Components and Engine.Reflow.Components (component-to-model injection table), drop when model injection is registered by the component bundles
# shortcut: RenderSceneManager.Inject.cpp uses Engine.Render.Scene.Model (CameraModel type mapping), drop when model injection is registered by Engine.Render.Scene.Model