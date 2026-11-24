# Configure support libraries
set(__support_libraries "")
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
	list(APPEND __support_libraries ${PROJECT_NAME}::Support.Platform.Win32.Module ${PROJECT_NAME}::Support.Input.Win32.Module)
endif ()


# Setup
make_library_project(
        LIB_NAME Engine.Input
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/Input"
        LIBRARIES
        ${__support_libraries}
        	${PROJECT_NAME}::Engine.Input.Schedule.Module
        	${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Platform
)
# shortcut: Input.cpp imports Engine.Input.Schedule, Support.Input.Win32 and Support.Platform.Win32, and each of them links Engine.Input, drop when Input no longer names its schedule and Win32 backends