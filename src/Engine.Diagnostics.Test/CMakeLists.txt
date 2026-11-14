# Setup
make_test_project(
		TEST_NAME Engine.Diagnostics.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Diagnostics
		${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Pedantic
)
