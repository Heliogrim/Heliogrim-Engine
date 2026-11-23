
# Setup
make_library_project(
		LIB_NAME Engine.Physics.Schedule
		MODULE
		LIB_FOLDER "${IDE_FOLDER}/Engine/Physics"
		LIBRARIES ${PROJECT_NAME}::Engine.Serialization
			${PROJECT_NAME}::Engine.Core.Schedule.Module ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Physics ${PROJECT_NAME}::Engine.Scene ${PROJECT_NAME}::Engine.Scene.Support ${PROJECT_NAME}::Engine.Physics.Scene ${PROJECT_NAME}::Engine.Scene.Schedule.Module
)
# shortcut: PhysicsTickStage.cpp imports Engine.Scene.Schedule (ScenePipeline as stage-ordering anchor) and Engine.Scene.Schedule links Physics.Schedule, drop when pipeline ordering no longer names the other pipelines