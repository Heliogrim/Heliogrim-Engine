
# Setup
make_library_project(
        LIB_NAME "Editor.Graphics.IO"
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Editor/IO"
        LIBRARIES ${PROJECT_NAME}::Editor.Graphics.Loader ${PROJECT_NAME}::Engine.Resource ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Editor.Graphics.Event ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Graphics.Core
)
