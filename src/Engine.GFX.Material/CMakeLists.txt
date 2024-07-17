
# Setup
make_library_project(
        LIB_NAME Engine.GFX.Material
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)