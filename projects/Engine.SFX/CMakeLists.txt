
# Setup
make_library_project(
        LIB_NAME Engine.SFX
        LIB_FOLDER "${IDE_FOLDER}/Engine/Audio"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)