
# Setup
make_library_project(
        LIB_NAME Engine.Reflect
        LIB_FOLDER "${IDE_FOLDER}/Engine/Utils"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)