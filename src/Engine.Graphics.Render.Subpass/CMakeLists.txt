
# Setup
make_library_project(
		LIB_NAME Engine.Graphics.Render.Subpass
		LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
		LIBRARIES
		${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Resource ${PROJECT_NAME}::Engine.Graphics.RenderGraph ${PROJECT_NAME}::Engine.Graphics.Render.Graph.Scene
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Async
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Config
		${PROJECT_NAME}::Engine.Core
		${PROJECT_NAME}::Engine.Graphics.Scene
		${PROJECT_NAME}::Engine.Reflect
		${PROJECT_NAME}::Engine.Reflect.Utils
		${PROJECT_NAME}::Engine.Render.Scene
		${PROJECT_NAME}::Engine.Storage
		${PROJECT_NAME}::Engine.Storage.System
		${PROJECT_NAME}::vulkan.Module
		${PROJECT_NAME}::Engine.Pedantic.Resource ${PROJECT_NAME}::Engine.Graphics.Types ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Storage.Action ${PROJECT_NAME}::Engine.Assets.Type
		${PROJECT_NAME}::Engine.Accel.Pass ${PROJECT_NAME}::Engine.Graphics.Loader ${PROJECT_NAME}::Engine.Graphics.Scene.Model ${PROJECT_NAME}::Engine.Accel.Lang
)
# shortcut: Engine.Graphics.Render.Subpass .cpp files reach the graphics front via getEngine()->getGraphics(), drop when the service-locator design lands