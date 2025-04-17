
# Setup
make_test_project(
		TEST_NAME Engine.GFX.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.GFX
		${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::Heliogrim
)