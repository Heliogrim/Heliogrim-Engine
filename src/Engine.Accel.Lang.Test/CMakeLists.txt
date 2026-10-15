# Setup
make_test_project(
		TEST_NAME Engine.Accel.Lang.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Accel.Lang
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Pedantic
		${PROJECT_NAME}::Engine.Reflect
)
