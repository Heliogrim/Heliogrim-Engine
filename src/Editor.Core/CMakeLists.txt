
# Setup
make_library_project(
		LIB_NAME "Editor.Core"
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Config ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Editor.Boot.Module ${PROJECT_NAME}::Engine.Network ${PROJECT_NAME}::Engine.Physics ${PROJECT_NAME}::Engine.Platform
)