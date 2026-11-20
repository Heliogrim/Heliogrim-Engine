
# Setup
make_library_project(
        LIB_NAME Engine.Input.Schedule
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Core
)