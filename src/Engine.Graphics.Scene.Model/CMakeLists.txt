
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.Scene.Model
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)