
# Setup
make_library_project(
        LIB_NAME Engine.Audio
        LIB_FOLDER "${IDE_FOLDER}/Engine/Audio"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)