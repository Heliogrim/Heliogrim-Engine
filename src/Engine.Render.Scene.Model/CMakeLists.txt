
# Setup
make_library_project(
        LIB_NAME Engine.Render.Scene.Model
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Graphics.Scene ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Graphics.Components ${PROJECT_NAME}::Engine.Scene ${PROJECT_NAME}::Engine.Render.Scene
)