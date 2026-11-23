
# Setup
make_library_project(
        LIB_NAME Editor.UI.Main
        LIB_FOLDER "${IDE_FOLDER}/Editor"
        LIBRARIES ${PROJECT_NAME}::Engine.Level
       
        ${PROJECT_NAME}::Editor.Boot.Module
        ${PROJECT_NAME}::Engine.ACS
        ${PROJECT_NAME}::Editor.UI
        ${PROJECT_NAME}::Editor.UI.Init
        ${PROJECT_NAME}::Editor.UI.Rx
        ${PROJECT_NAME}::Engine.Async
        ${PROJECT_NAME}::Engine.Common
        ${PROJECT_NAME}::Engine.Core
        ${PROJECT_NAME}::Heliogrim
)
