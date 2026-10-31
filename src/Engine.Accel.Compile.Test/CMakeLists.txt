
# Setup
# shortcut: Engine.Accel.Compile/Storage are object libraries whose objects pull in the GFX backend, so the
# header-only key code links against Lang only; link the bundles (or the engine) once a test calls their .cpp code
make_test_project(
		TEST_NAME Engine.Accel.Compile.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Accel.Lang
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
)
