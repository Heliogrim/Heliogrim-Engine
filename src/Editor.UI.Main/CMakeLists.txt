
# Setup
make_library_project(
        LIB_NAME Editor.UI.Main
        LIB_FOLDER "${IDE_FOLDER}/Editor"
        LIBRARIES
        ${PROJECT_NAME}::Editor.UI
        ${PROJECT_NAME}::Editor.UI.Init
        ${PROJECT_NAME}::Heliogrim
)
