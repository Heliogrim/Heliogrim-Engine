
# Setup
make_library_project(
		LIB_NAME Engine.Accel.Pipeline
		MODULE
		LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Graphics.Types
		${PROJECT_NAME}::Engine.Graphics.Core ${PROJECT_NAME}::Engine.Accel.Lang ${PROJECT_NAME}::Engine.Accel.Effect
)