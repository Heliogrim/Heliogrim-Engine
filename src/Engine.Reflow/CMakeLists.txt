
# Setup
make_library_project(
        LIB_NAME Engine.Reflow
        MODULE
        MODULE_EXCLUDE
        # Unused and stale: names `input_type` from a dependent base without `typename`.
        Engine.Reflow/Widget/Input/InputVector.hpp
        LIB_FOLDER "${IDE_FOLDER}/Engine/UI"
        LIBRARIES
        	${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.Scene ${PROJECT_NAME}::Engine.Render.Scene ${PROJECT_NAME}::Engine.Reflow.Schedule
        	${PROJECT_NAME}::Engine.Input ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Graphics.Scene ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Platform ${PROJECT_NAME}::Engine.Storage.Registry ${PROJECT_NAME}::Engine.Graphics.Loader
        	${PROJECT_NAME}::Engine.Graphics.Core ${PROJECT_NAME}::Engine.Graphics.Render
)
# shortcut: Scene/UISceneModel.cpp uses Engine.Reflow.Components (scene-model downcast to UIComponent), drop when the model moves next to the component
