
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.Types
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/Graphics"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)
