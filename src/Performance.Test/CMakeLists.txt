
# Setup
make_test_project(
		TEST_NAME Performance.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Resource.Archive
		${PROJECT_NAME}::Heliogrim
)