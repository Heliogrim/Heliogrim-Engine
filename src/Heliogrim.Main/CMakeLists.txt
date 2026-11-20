
# Setup
make_library_project(
        LIB_NAME Heliogrim.Main
        LIB_FOLDER "${IDE_FOLDER}"
        LIBRARIES
        ${PROJECT_NAME}::Engine.Common
        ${PROJECT_NAME}::Engine.Core
        ${PROJECT_NAME}::Engine.Core.Game
        ${PROJECT_NAME}::Engine.Event
        ${PROJECT_NAME}::Heliogrim
)