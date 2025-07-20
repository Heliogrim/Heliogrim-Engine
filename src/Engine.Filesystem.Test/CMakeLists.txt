
# Setup
make_test_project(
		TEST_NAME Engine.Filesystem.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Filesystem
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Core
		${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::Engine.Scheduler
		${PROJECT_NAME}::Heliogrim
)