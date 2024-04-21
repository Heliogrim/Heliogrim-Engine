
# Setup
make_library_project(
        LIB_NAME Engine.Core
        LIB_FOLDER "${IDE_FOLDER}/Engine/Core"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)