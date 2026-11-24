
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.Render
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.Resource ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Storage ${PROJECT_NAME}::Engine.Input ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Graphics.Scene ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Reflect.Utils ${PROJECT_NAME}::Engine.Config
        ${PROJECT_NAME}::Engine.IO.Dispatcher
        ${PROJECT_NAME}::Engine.Pedantic.Resource ${PROJECT_NAME}::Engine.Graphics.Types ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Storage.Action ${PROJECT_NAME}::Engine.Graphics.Loader ${PROJECT_NAME}::Engine.Accel.Lang ${PROJECT_NAME}::Engine.Accel.Effect ${PROJECT_NAME}::Engine.Graphics.RenderGraph.Compile ${PROJECT_NAME}::Engine.Graphics.RenderGraph ${PROJECT_NAME}::Engine.Graphics.Render.Graph.Scene ${PROJECT_NAME}::Engine.Graphics.Render.Predefined.Module ${PROJECT_NAME}::Engine.Reflow.Render.Module ${PROJECT_NAME}::Engine.Graphics.Render.Subpass ${PROJECT_NAME}::Engine.Graphics.Core.Module
)
# shortcut: RenderPass.cpp, RenderTarget.cpp and __Test__Renderer.cpp import Engine.Graphics.Render.Predefined, linked as Engine.Graphics.Render.Predefined.Module because Engine.Graphics.Render.Predefined transitively links Engine.Graphics.Render via Engine.Accel.Pipeline, drop when Engine.Accel.Pipeline stops linking Engine.Graphics.Core
# shortcut: Engine.Graphics.Render .cpp files reach the graphics front via getEngine()->getGraphics(), drop when the service-locator design lands
# shortcut: __Test__Renderer.cpp imports Engine.Reflow.Render, linked as Engine.Reflow.Render.Module because Engine.Reflow.Render transitively links Engine.Graphics.Render via Engine.Graphics.Core, drop when Engine.Graphics.Core stops linking Engine.Graphics.Render
# shortcut: RenderPass.cpp imports Engine.Graphics.Core, linked as Engine.Graphics.Core.Module because Engine.Graphics.Core links Engine.Graphics.Render, drop when the service-locator design lands