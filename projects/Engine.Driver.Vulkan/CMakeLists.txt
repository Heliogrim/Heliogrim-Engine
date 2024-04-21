
# Setup
make_library_project(
        LIB_NAME Engine.Driver.Vulkan
        LIB_FOLDER "${IDE_FOLDER}/Driver"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)