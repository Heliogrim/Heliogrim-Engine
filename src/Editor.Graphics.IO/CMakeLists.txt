
# Setup
make_library_project(
        LIB_NAME "Editor.Graphics.IO"
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Editor/IO"
        LIBRARIES ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Editor.Graphics.Event ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Graphics.Event
)
