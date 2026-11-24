
# Setup
make_library_project(
		LIB_NAME Engine.Graphics.Render.Subpass
		MODULE
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
		${PROJECT_NAME}::Engine.Accel.Pass ${PROJECT_NAME}::Engine.Graphics.Loader ${PROJECT_NAME}::Engine.Graphics.Scene.Model ${PROJECT_NAME}::Engine.Accel.Lang ${PROJECT_NAME}::Engine.Accel.Effect ${PROJECT_NAME}::Engine.Accel.Pipeline.Module ${PROJECT_NAME}::Engine.Accel.Command.Module
		${PROJECT_NAME}::Engine.Accel.Compile.Module ${PROJECT_NAME}::Engine.Graphics.Render.Command.Module ${PROJECT_NAME}::Engine.Graphics.Render.Predefined.Module ${PROJECT_NAME}::Engine.Graphics.Material.Module ${PROJECT_NAME}::Engine.Driver.Vulkan.Module ${PROJECT_NAME}::Engine.Graphics.Core.Module
)
# shortcut: Impl/*Pass.cpp, Impl/DummyProvider.cpp and Impl/Visualize.cpp import Engine.Accel.Pipeline and Engine.Accel.Command, linked as .Module targets because Engine.Accel.Pipeline (and Engine.Accel.Command through it) transitively links Engine.Graphics.Render.Subpass, drop when Engine.Accel.Pipeline stops linking Engine.Graphics.Core
# shortcut: Impl/*Pass.{hpp,cpp}, Impl/DummyProvider.cpp and Impl/Visualize.{hpp,cpp} import Engine.Accel.Compile, Engine.Graphics.Render.Command and Engine.Graphics.Render.Predefined, linked as .Module targets because each transitively links Engine.Graphics.Render.Subpass via Engine.Accel.Pipeline and Engine.Graphics.Core, drop when Engine.Accel.Pipeline stops linking Engine.Graphics.Core
# shortcut: Impl/*Pass.cpp, Impl/DummyProvider.cpp and Impl/Visualize.cpp import Engine.Driver.Vulkan, linked as Engine.Driver.Vulkan.Module because Engine.Driver.Vulkan links Engine.Graphics.Core, which links Engine.Graphics.Render.Subpass, drop when Engine.Driver.Vulkan stops linking Engine.Graphics.Core
# shortcut: Engine.Graphics.Render.Subpass .cpp files reach the graphics front via getEngine()->getGraphics(), drop when the service-locator design lands
# shortcut: Impl/*Pass.cpp, Impl/DummyProvider.cpp and Impl/Visualize.cpp import Engine.Graphics.Core, linked as Engine.Graphics.Core.Module because Engine.Graphics.Core links Engine.Graphics.Render.Subpass, drop when the service-locator design lands
# shortcut: Impl/SkyBoxPass.hpp imports Engine.Graphics.Material, linked as Engine.Graphics.Material.Module because Engine.Graphics.Material transitively links Engine.Graphics.Render.Subpass via Engine.Graphics.Render.Predefined, Engine.Accel.Pipeline and Engine.Graphics.Core, drop when Engine.Accel.Pipeline stops linking Engine.Graphics.Core