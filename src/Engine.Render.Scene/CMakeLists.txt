
# Setup
make_library_project(
        LIB_NAME Engine.Render.Scene
        MODULE
        LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Scene ${PROJECT_NAME}::Engine.Graphics.Types ${PROJECT_NAME}::Engine.Graphics.RenderGraph.Module
)
# shortcut: Engine.Render.Scene .cpp files reach the graphics front via getEngine()->getGraphics(), drop when the service-locator design lands
# shortcut: RenderSceneSystem.cpp includes Engine.Graphics.Core/Graphics.hpp, whose Engine.Graphics.Render/Renderer.hpp imports Engine.Graphics.RenderGraph, linked as Engine.Graphics.RenderGraph.Module because Engine.Graphics.RenderGraph transitively links Engine.Render.Scene, drop when the service-locator design lands