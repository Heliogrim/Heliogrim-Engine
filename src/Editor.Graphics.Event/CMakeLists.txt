
# Setup
make_library_project(
        LIB_NAME "Editor.Graphics.Event"
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Editor/Event"
        LIBRARIES ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Event
)
