
# Setup
make_library_project(
		LIB_NAME Engine.Async
		LIBRARIES
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Scheduler
)