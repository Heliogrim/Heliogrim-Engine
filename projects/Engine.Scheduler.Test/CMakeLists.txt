
# Setup
make_test_project(
		TEST_NAME Engine.Scheduler.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Scheduler
)