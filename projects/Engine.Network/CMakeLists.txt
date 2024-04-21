
# Setup
make_library_project(
        LIB_NAME Engine.Network
        LIB_FOLDER "${IDE_FOLDER}/Engine/Network"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)