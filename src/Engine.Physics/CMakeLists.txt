
# Setup
make_library_project(
        LIB_NAME Engine.Physics
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/Physics"
        LIBRARIES ${PROJECT_NAME}::Engine.Physics.Schedule.Module ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core
)
# shortcut: Physics.cpp imports Engine.Physics.Schedule and Engine.Physics.Schedule links Engine.Physics, drop when Physics no longer names its schedule