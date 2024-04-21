
# Setup
make_library_project(
        LIB_NAME Engine.Pedantic
        LIB_FOLDER "${IDE_FOLDER}/Engine/Utils"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)