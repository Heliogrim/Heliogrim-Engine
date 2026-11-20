
# Setup
make_library_project(
        LIB_NAME Engine.Reflow
        LIB_FOLDER "${IDE_FOLDER}/Engine/UI"
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::vulkan.Module
)