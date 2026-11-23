
# Setup
make_test_project(
		TEST_NAME Engine.Reflow.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Reflow ${PROJECT_NAME}::Engine.Reflow.Uikit
		${PROJECT_NAME}::Heliogrim
		${PROJECT_NAME}::Engine.Assets
)