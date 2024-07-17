
# Setup
make_library_project(
        LIB_NAME Engine.Accel
        LIB_FOLDER "${IDE_FOLDER}/Engine/Accel"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)