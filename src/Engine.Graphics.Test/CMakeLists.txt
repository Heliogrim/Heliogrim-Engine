
# Setup
make_test_project(
		TEST_NAME Engine.Graphics.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Graphics
		${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::Heliogrim
)