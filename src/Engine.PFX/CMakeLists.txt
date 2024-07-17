
# Setup
make_library_project(
        LIB_NAME Engine.PFX
        LIB_FOLDER "${IDE_FOLDER}/Engine/Physics"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)