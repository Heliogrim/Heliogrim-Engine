
# Setup
make_library_project(
        LIB_NAME Engine.Input
        LIB_FOLDER "${IDE_FOLDER}/Engine/Input"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)