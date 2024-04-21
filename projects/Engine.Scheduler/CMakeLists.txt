
# Setup
make_library_project(
        LIB_NAME Engine.Scheduler
        LIB_FOLDER "${IDE_FOLDER}/Engine/Schedule"
        LIBRARIES ${PROJECT_NAME}::Engine.Common
)