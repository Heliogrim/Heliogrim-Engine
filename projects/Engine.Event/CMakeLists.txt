
# Setup
make_library_project(
        LIB_NAME Engine.Event
        LIB_FOLDER "${IDE_FOLDER}/Engine/Event"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)