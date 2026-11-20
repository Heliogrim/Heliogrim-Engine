
# Setup
make_library_project(
		LIB_NAME Engine.Graphics.Render.Subpass
		LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
		LIBRARIES
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Async
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Config
		${PROJECT_NAME}::Engine.Core
		${PROJECT_NAME}::Engine.Graphics.Scene
		${PROJECT_NAME}::Engine.Reflect
		${PROJECT_NAME}::Engine.Reflect.Utils
		${PROJECT_NAME}::Engine.Storage
		${PROJECT_NAME}::Engine.Storage.System
		${PROJECT_NAME}::vulkan.Module
		${PROJECT_NAME}::Engine.Pedantic.Resource
)