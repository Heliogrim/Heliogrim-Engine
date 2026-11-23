
# Setup
make_library_project(
		LIB_NAME Engine.Physics.Schedule
		MODULE
		LIB_FOLDER "${IDE_FOLDER}/Engine/Physics"
		LIBRARIES ${PROJECT_NAME}::Engine.Serialization
			${PROJECT_NAME}::Engine.Core.Schedule.Module ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Physics ${PROJECT_NAME}::Engine.Scene ${PROJECT_NAME}::Engine.Scene.Support
)