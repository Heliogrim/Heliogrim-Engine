
# Setup
make_library_project(
        LIB_NAME Heliogrim.Main
        LIB_FOLDER "${IDE_FOLDER}"
        LIBRARIES
        ${PROJECT_NAME}::Heliogrim
        ${PROJECT_NAME}::Engine.Core.Game
)