
# Setup
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
	make_library_project(
			LIB_NAME Support.Input.Win32
			MODULE
			MODULE_EXCLUDE
			# Hoisted into the global fragment otherwise, where its Engine.Input import has no effect.
			Support.Input.Win32/DragDrop/Win32DragDropReceiver.hpp
			LIB_FOLDER "${IDE_FOLDER}/Support"
			LIBRARIES
			${PROJECT_NAME}::Engine.Async
			${PROJECT_NAME}::Engine.Scheduler
			${PROJECT_NAME}::Engine.Asserts
			${PROJECT_NAME}::Engine.Common
			${PROJECT_NAME}::Engine.Core
			${PROJECT_NAME}::Engine.Input
			${PROJECT_NAME}::Engine.Logging
			${PROJECT_NAME}::Engine.Platform
			${PROJECT_NAME}::Engine.Reflect
	)
endif ()