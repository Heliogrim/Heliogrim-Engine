
# Setup
make_library_project(
        LIB_NAME "Editor.Graphics.Loader"
        LIB_FOLDER "${IDE_FOLDER}/Editor/IO"
        LIBRARIES ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core
)
