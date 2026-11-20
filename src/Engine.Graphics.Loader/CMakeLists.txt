
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.Loader
        LIB_FOLDER "${IDE_FOLDER}/Engine/IO"
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics
)