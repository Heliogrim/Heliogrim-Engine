
# Setup
make_library_project(
        LIB_NAME "Editor.Graphics.Loader"
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Editor/IO"
        LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.Assets.System ${PROJECT_NAME}::Engine.Resource ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Storage.Action ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Storage.System ${PROJECT_NAME}::Engine.Graphics.Loader
)
