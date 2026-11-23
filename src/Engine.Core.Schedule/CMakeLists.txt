
# Setup
make_library_project(
        LIB_NAME Engine.Core.Schedule
        MODULE
        LIBRARIES
        	${PROJECT_NAME}::Engine.ACS.Schedule.Module ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core
)