
# Setup
# Engine.Accel.Compile is an object library whose objects pull in the GFX backend, so the test links the engine
# the way the GFX test does, to resolve the symbols of the bundle code it calls
make_test_project(
		TEST_NAME Engine.Accel.Compile.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Accel.Compile
		${PROJECT_NAME}::Engine.Accel.Lang
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Graphics
		${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::Heliogrim
)
