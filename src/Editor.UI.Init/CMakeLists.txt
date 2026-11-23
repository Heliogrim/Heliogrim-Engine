
# Setup
make_library_project(
        LIB_NAME Editor.UI.Init
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Editor"
        LIBRARIES
        ${PROJECT_NAME}::Editor.UI
        ${PROJECT_NAME}::Engine.ACS
        ${PROJECT_NAME}::Editor.UI.Rx
        ${PROJECT_NAME}::Engine.Common
        ${PROJECT_NAME}::Heliogrim
        ${PROJECT_NAME}::Engine.Reflow.Components
        ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type
        ${PROJECT_NAME}::Engine.Graphics.Core ${PROJECT_NAME}::Engine.Graphics.Render
)
