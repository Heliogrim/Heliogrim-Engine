
# Setup
make_library_project(
        LIB_NAME Engine.Platform
        LIB_FOLDER "${IDE_FOLDER}/Platform"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)