
# Setup
make_library_project(
        LIB_NAME Editor.UI.Main
        LIB_FOLDER "${IDE_FOLDER}/Editor"
        LIBRARIES ${PROJECT_NAME}::Editor.Assets.Default ${PROJECT_NAME}::Engine.Assets.System ${PROJECT_NAME}::Engine.Resource ${PROJECT_NAME}::Engine.Level
       
        ${PROJECT_NAME}::Editor.Boot.Module ${PROJECT_NAME}::Editor.Core
        ${PROJECT_NAME}::Engine.ACS
        ${PROJECT_NAME}::Editor.UI
        ${PROJECT_NAME}::Editor.UI.Init ${PROJECT_NAME}::Editor.UI.Theme ${PROJECT_NAME}::Editor.UI.View
        ${PROJECT_NAME}::Editor.UI.Rx
        ${PROJECT_NAME}::Engine.Async
        ${PROJECT_NAME}::Engine.Common
        ${PROJECT_NAME}::Engine.Core
        ${PROJECT_NAME}::Heliogrim
        ${PROJECT_NAME}::Engine.Assets
        ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Reflow ${PROJECT_NAME}::Engine.Graphics.Loader
)
# shortcut: EditorUI.cpp imports Editor.Boot, linked as Editor.Boot.Module because Editor.Boot links Editor.UI.Main, drop when EditorUI.cpp no longer imports Editor.Boot
