
# Setup
make_library_project(
		LIB_NAME Engine.Accel.Pipeline
		LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics
)