
# Setup
make_library_project(
		LIB_NAME Engine.Accel.Pass
		LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Graphics.Types ${PROJECT_NAME}::Engine.Accel.Lang
)
# shortcut: Engine.Accel.Pass .cpp files reach the graphics front via getEngine()->getGraphics(), drop when the service-locator design lands