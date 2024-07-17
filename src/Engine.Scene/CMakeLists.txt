
# Setup
make_library_project(
        LIB_NAME Engine.Scene
        LIB_FOLDER "${IDE_FOLDER}/Engine/Scene"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)