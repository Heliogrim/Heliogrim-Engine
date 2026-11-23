# Setup
make_library_project(
        LIB_NAME Engine.Reflow.Components
        LIB_FOLDER "${IDE_FOLDER}/Engine/UI"
        LIBRARIES ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Reflow ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect
)
