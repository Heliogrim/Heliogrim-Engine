
# Setup
make_library_project(
        LIB_NAME Engine.Input.Schedule
        MODULE
        LIBRARIES
        	${PROJECT_NAME}::Engine.Core.Schedule.Module
        	${PROJECT_NAME}::Engine.Input.Module ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Core
        ${PROJECT_NAME}::Engine.Platform
)