
# Setup
make_library_project(
        LIB_NAME Editor.Assets.Default
        LIB_FOLDER "${IDE_FOLDER}/Editor/Assets"
        LIBRARIES ${PROJECT_NAME}::Heliogrim
)
