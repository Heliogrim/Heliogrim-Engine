
# Setup
# Engine.Accel.Compile is an object library whose objects pull in the graphics backend, so the test links the engine
# the way the Graphics test does, to resolve the symbols of the bundle code it calls
make_test_project(
		TEST_NAME Engine.Accel.Compile.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Accel.Compile
		${PROJECT_NAME}::Engine.Accel.Effect
		${PROJECT_NAME}::Engine.Accel.Lang
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Diagnostics
		${PROJECT_NAME}::Engine.Graphics
		${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::External-spirv-opt
		${PROJECT_NAME}::Heliogrim
		${PROJECT_NAME}::vulkan.Module
)

name_to_target(__compile_test_target "Engine.Accel.Compile.Test")
target_compile_definitions(${__compile_test_target} PRIVATE HG_TEST_SHADER_ROOT="${CMAKE_SOURCE_DIR}/assets/shader")
