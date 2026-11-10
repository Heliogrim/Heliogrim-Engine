
# Setup
make_library_project(
        LIB_NAME Engine.Physics
        LIB_FOLDER "${IDE_FOLDER}/Engine/Physics"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)