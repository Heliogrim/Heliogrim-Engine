
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.Material
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)