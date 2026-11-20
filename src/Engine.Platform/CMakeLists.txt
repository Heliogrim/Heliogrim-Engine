
# Setup
make_library_project(
        LIB_NAME Engine.Platform
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Platform"
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Core
)