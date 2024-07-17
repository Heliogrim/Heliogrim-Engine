
# Setup
make_library_project(
        LIB_NAME Engine.Config
        LIB_FOLDER "${IDE_FOLDER}/Engine"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)