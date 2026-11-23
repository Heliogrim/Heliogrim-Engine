
# Setup
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
	make_library_project(
			LIB_NAME Support.Platform.Win32
			MODULE
			LIB_FOLDER "${IDE_FOLDER}/Support"
			LIBRARIES
				${PROJECT_NAME}::Engine.Input.Module
			${PROJECT_NAME}::Engine.Scheduler
			${PROJECT_NAME}::Engine.Asserts
			${PROJECT_NAME}::Engine.Async
			${PROJECT_NAME}::Engine.Common
			${PROJECT_NAME}::Engine.Core
			${PROJECT_NAME}::Engine.Event
			${PROJECT_NAME}::Engine.Logging
			${PROJECT_NAME}::Engine.Platform ${PROJECT_NAME}::Engine.Graphics.Types
			${PROJECT_NAME}::vulkan.Module
	)
endif ()
# shortcut: Support.Platform.Win32 .cpp files reach the graphics front via getEngine()->getGraphics(), drop when the service-locator design lands