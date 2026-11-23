
# Setup
make_library_project(
        LIB_NAME Engine.Core
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/Core"
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Config ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive
)
# shortcut: Universe.cpp textually includes Engine.Level (an upward include) and so imports Engine.Async; drop when Universe stops including Level
target_link_libraries(${PROJECT_NAME}-Engine.Core PRIVATE ${PROJECT_NAME}::Engine.Async.Module)