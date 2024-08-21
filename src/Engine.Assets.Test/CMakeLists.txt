
# Setup
make_test_project(
		TEST_NAME Engine.Assets.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.ACS
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Assets
		${PROJECT_NAME}::Engine.Assets.System
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Core
		${PROJECT_NAME}::Engine.Filesystem
		${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::Engine.Resource.Archive
		${PROJECT_NAME}::Engine.Scheduler
		${PROJECT_NAME}::Engine.Serialization
		${PROJECT_NAME}::Engine.Serialization.Layouts
		${PROJECT_NAME}::Engine.Serialization.Structures
		${PROJECT_NAME}::Heliogrim
)