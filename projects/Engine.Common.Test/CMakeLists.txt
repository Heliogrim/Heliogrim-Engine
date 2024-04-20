
# Setup
make_test_project(
		TEST_NAME Engine.Common.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Scheduler
)