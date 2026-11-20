
# Setup
make_library_project(
        LIB_NAME Engine.Render.Scene.Model
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Graphics.Scene
)