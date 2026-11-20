
# Setup
make_library_project(
        LIB_NAME Engine.Core
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/Core"
        LIBRARIES ${PROJECT_NAME}::Engine.Scheduler.Module ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive
)