
# Setup
make_library_project(
		LIB_NAME Engine.Scene.Schedule
		LIBRARIES
			${PROJECT_NAME}::Engine.Physics.Schedule
			${PROJECT_NAME}::Engine.Input.Schedule
			${PROJECT_NAME}::Engine.Core.Schedule
			${PROJECT_NAME}::Engine.ACS.Schedule ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core
)