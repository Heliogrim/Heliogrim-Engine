
# Setup
make_library_project(
        LIB_NAME Editor.Render
        LIB_FOLDER "${IDE_FOLDER}/Editor"
        LIBRARIES ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Graphics.RenderGraph ${PROJECT_NAME}::Engine.Graphics.Render.Predefined
)
