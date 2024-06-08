
# Setup
make_app_project(
        APP_NAME "Editor"
        APP_FOLDER "${IDE_FOLDER}/Editor"
        LIB_NAME "Editor.Main"
        LIB_FOLDER "${IDE_FOLDER}/Editor"
        SOURCE "Editor.Main"
        LIBRARIES
        ${PROJECT_NAME}::Editor.Action
        ${PROJECT_NAME}::Editor.Assets.Default
        ${PROJECT_NAME}::Editor.Core
        ${PROJECT_NAME}::Editor.GFX.Event
        ${PROJECT_NAME}::Editor.GFX.Graphs
        ${PROJECT_NAME}::Editor.GFX.IO
        ${PROJECT_NAME}::Editor.GFX.Loader
        ${PROJECT_NAME}::Editor.Render
        ${PROJECT_NAME}::Editor.Scene
        ${PROJECT_NAME}::Editor.UI
        ${PROJECT_NAME}::Editor.UI.Init
        ${PROJECT_NAME}::Editor.UI.Main
        ${PROJECT_NAME}::Heliogrim
)