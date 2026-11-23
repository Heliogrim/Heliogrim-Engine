
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.Render.Graph.Scene
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Graphics.Scene ${PROJECT_NAME}::Engine.Render.Scene
)