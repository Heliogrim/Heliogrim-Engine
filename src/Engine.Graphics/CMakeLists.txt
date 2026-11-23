# Configure support libraries
set(__support_libraries "")
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
	list(APPEND __support_libraries ${PROJECT_NAME}::Support.Platform.Win32)
endif ()


# Setup
make_library_project(
        LIB_NAME Engine.Graphics
        MODULE
        MODULE_EXCLUDE
        # Unused legacy macro header; it includes the excluded Engine.Common/stdafx.h.
        Engine.Graphics/Inc.hpp
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES
        ${__support_libraries} ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Resource.Package ${PROJECT_NAME}::Engine.Resource ${PROJECT_NAME}::Engine.Assets.Database
        	${PROJECT_NAME}::Engine.Serialization
        	${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Platform
        ${PROJECT_NAME}::Engine.Pedantic.Resource ${PROJECT_NAME}::Engine.Graphics.Types ${PROJECT_NAME}::Engine.Storage.Registry ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Resource ${PROJECT_NAME}::Engine.Graphics.RenderGraph.Module
)
# shortcut: Engine.Graphics .cpp files reach the graphics front via getEngine()->getGraphics(), drop when the service-locator design lands
# shortcut: Buffer/SparseBuffer.cpp, Geometry/Cube.cpp, Geometry/Plane.cpp and Texture/SparseTexture.cpp include Engine.Graphics.Render/Renderer.hpp (via Engine.Graphics.Core/Graphics.hpp or Device/Device.hpp), which imports Engine.Graphics.RenderGraph, linked as Engine.Graphics.RenderGraph.Module because Engine.Graphics.RenderGraph links Engine.Graphics, drop when the service-locator design lands