
# Setup
make_library_project(
        LIB_NAME Editor.Scene
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Editor"
        LIBRARIES ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Scene ${PROJECT_NAME}::Engine.Scene.Game ${PROJECT_NAME}::Engine.Scene.Preview
)
