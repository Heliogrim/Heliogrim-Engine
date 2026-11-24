
# Setup
make_library_project(
        LIB_NAME Editor.UI.Init
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Editor"
        LIBRARIES ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Editor.Core
        ${PROJECT_NAME}::Editor.UI ${PROJECT_NAME}::Editor.UI.Theme ${PROJECT_NAME}::Editor.UI.View ${PROJECT_NAME}::Engine.Reflow.Uikit
        ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Graphics.Render
        ${PROJECT_NAME}::Editor.UI.Rx
        ${PROJECT_NAME}::Engine.Common
        ${PROJECT_NAME}::Heliogrim
        ${PROJECT_NAME}::Engine.Reflow.Components ${PROJECT_NAME}::Engine.Reflow
        ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type
        ${PROJECT_NAME}::Engine.Graphics.Core ${PROJECT_NAME}::Engine.Graphics.Render
)
# shortcut: EditorInit.cpp imports Editor.UI.Main (initEditor fills the EditorUI model) and Editor.UI.Main links Editor.UI.Init, drop when init moves into Editor.UI.Main
target_link_libraries(${PROJECT_NAME}-Editor.UI.Init PRIVATE ${PROJECT_NAME}::Editor.UI.Main.Module)
