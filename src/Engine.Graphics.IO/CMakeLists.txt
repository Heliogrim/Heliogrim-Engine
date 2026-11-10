
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.IO
        LIB_FOLDER "${IDE_FOLDER}/Engine/IO"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)