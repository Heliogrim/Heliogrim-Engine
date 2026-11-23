# Configure support libraries
set(__support_libraries "")
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
	list(APPEND __support_libraries ${PROJECT_NAME}::Support.Platform.Win32)
endif ()


# Setup
make_library_project(
        LIB_NAME Engine.Graphics
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES
        ${__support_libraries} ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Resource.Package ${PROJECT_NAME}::Engine.Scene ${PROJECT_NAME}::Engine.Render.Scene ${PROJECT_NAME}::Engine.Resource ${PROJECT_NAME}::Engine.Assets.Database
        	${PROJECT_NAME}::Engine.Graphics.Schedule
        	${PROJECT_NAME}::Engine.Serialization
        	${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Graphics.Scene ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Platform
        ${PROJECT_NAME}::Engine.Pedantic.Resource ${PROJECT_NAME}::Engine.Graphics.Types ${PROJECT_NAME}::Engine.Storage.Registry ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Resource
        ${PROJECT_NAME}::Engine.Accel.Pass ${PROJECT_NAME}::Engine.Graphics.Loader
)
# shortcut: Command/CommandBuffer.cpp uses Engine.Accel.Pass (framebuffer), drop when Graphics.cpp moves to Engine.Graphics.Core (Task 17)
# shortcut: Graphics.cpp uses Engine.Graphics.Loader (cache ctrl), drop when Graphics.cpp moves to Engine.Graphics.Core (Task 17)