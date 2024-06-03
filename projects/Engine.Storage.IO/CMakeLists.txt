
# Setup
make_library_project(
        LIB_NAME Engine.Storage.IO
        LIB_FOLDER "${IDE_FOLDER}/Engine/Storage.IO"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)