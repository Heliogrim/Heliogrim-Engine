
# Setup
make_test_project(
		TEST_NAME Engine.Reflow.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Reflow
		${PROJECT_NAME}::Heliogrim
)