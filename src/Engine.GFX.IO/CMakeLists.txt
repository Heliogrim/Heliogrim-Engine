
# Setup
make_library_project(
        LIB_NAME Engine.GFX.IO
        LIB_FOLDER "${IDE_FOLDER}/Engine/IO"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)