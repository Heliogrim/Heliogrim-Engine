
# Setup
make_test_project(
		TEST_NAME Engine.Scene.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Event
		${PROJECT_NAME}::Engine.Scene
		${PROJECT_NAME}::Engine.Scheduler
)