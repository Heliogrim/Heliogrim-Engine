
# Setup
make_library_project(
        LIB_NAME Engine.Assets
        LIB_FOLDER "${IDE_FOLDER}/Engine/Assets"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)