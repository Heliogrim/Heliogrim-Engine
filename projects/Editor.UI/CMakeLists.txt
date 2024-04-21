
# Setup
make_library_project(
        LIB_NAME Editor.UI
        LIB_FOLDER "${IDE_FOLDER}/Editor"
        LIBRARIES ${PROJECT_NAME}::Heliogrim
)
