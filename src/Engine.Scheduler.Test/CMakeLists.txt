
# Setup
make_test_project(
		TEST_NAME Engine.Scheduler.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.ACS
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Config
		${PROJECT_NAME}::Engine.Core
		${PROJECT_NAME}::Engine.Event
		${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::Engine.Scheduler
		${PROJECT_NAME}::Heliogrim
)