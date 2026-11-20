
# Setup
make_library_project(
		LIB_NAME Engine.Accel.Lang
		LIBRARIES
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Async
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Diagnostics
		${PROJECT_NAME}::Engine.Reflect
		${PROJECT_NAME}::External-spirv-headers
		${PROJECT_NAME}::External-spirv-opt
		${PROJECT_NAME}::External-spirv-static
)