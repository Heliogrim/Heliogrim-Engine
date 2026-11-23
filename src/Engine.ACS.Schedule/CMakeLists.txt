
# Setup
make_library_project(
		LIB_NAME Engine.ACS.Schedule
		MODULE
		LIBRARIES
			${PROJECT_NAME}::Engine.Physics.Schedule.Module
			${PROJECT_NAME}::Engine.Input.Schedule.Module
			${PROJECT_NAME}::Engine.Core.Schedule.Module ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Resource.Archive
)