
# Setup
make_library_project(
        LIB_NAME Editor.UI.Init
        LIB_FOLDER "${IDE_FOLDER}/Editor"
        LIBRARIES
        ${PROJECT_NAME}::Editor.UI
        ${PROJECT_NAME}::Heliogrim
)
