
# Setup
make_library_project(
        LIB_NAME Engine.Env
        LIB_FOLDER "${IDE_FOLDER}/Platform"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)