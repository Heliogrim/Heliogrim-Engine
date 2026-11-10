
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.Event
        LIB_FOLDER "${IDE_FOLDER}/Engine/Event"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)