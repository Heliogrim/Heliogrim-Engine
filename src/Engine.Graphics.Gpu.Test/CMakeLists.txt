
# Setup
# Headless GPU function harness; excluded from AllTests as it needs a device
make_test_project(
		TEST_NAME Engine.Graphics.Gpu.Test
		EXCLUDE_FROM_ALL_TESTS
		# CI on machines without a GPU skips these with `ctest -LE gpu`
		LABELS gpu
		LIBRARIES
		${PROJECT_NAME}::Engine.Accel.Compile
		${PROJECT_NAME}::Engine.Accel.Lang
		${PROJECT_NAME}::Engine.Accel.Pass
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Config
		${PROJECT_NAME}::Engine.Diagnostics
		${PROJECT_NAME}::Engine.Driver.Vulkan
		${PROJECT_NAME}::Engine.Event
		${PROJECT_NAME}::Engine.Graphics
		${PROJECT_NAME}::Engine.Graphics.RenderGraph
		${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::External-c4core
		${PROJECT_NAME}::External-ryml
		${PROJECT_NAME}::Heliogrim
)

name_to_target(__gpu_test_target "Engine.Graphics.Gpu.Test")
target_compile_definitions(${__gpu_test_target} PRIVATE HG_TEST_SHADER_ROOT="${CMAKE_SOURCE_DIR}/assets/shader")
