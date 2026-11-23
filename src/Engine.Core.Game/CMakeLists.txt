# Configure support libraries
set(__support_libraries "")
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
	list(APPEND __support_libraries ${PROJECT_NAME}::Support.Platform.Win32)
endif ()


# Setup
make_library_project(
        LIB_NAME Engine.Core.Game
        MODULE
        LIBRARIES
        ${__support_libraries}
        	${PROJECT_NAME}::Engine.Core.Schedule
        	${PROJECT_NAME}::Engine.ACS.Schedule
        	${PROJECT_NAME}::Engine.Scene.Schedule
        	${PROJECT_NAME}::Engine.ACS
        	${PROJECT_NAME}::Engine.Storage
        	${PROJECT_NAME}::Engine.Input ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Config ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Network ${PROJECT_NAME}::Engine.Physics ${PROJECT_NAME}::Engine.Platform ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Resource ${PROJECT_NAME}::Engine.Audio
)