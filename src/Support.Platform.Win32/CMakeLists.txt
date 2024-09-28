
# Setup
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
	make_library_project(
			LIB_NAME Support.Platform.Win32
			LIB_FOLDER "${IDE_FOLDER}/Support"
			LIBRARIES
			${PROJECT_NAME}::Engine.Common
			${PROJECT_NAME}::Engine.Platform
	)
endif ()