
# Setup
make_library_project(
		LIB_NAME Engine.ACS.Schedule
		MODULE
		LIBRARIES ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Level ${PROJECT_NAME}::Engine.Scene
			${PROJECT_NAME}::Engine.Physics.Schedule.Module
			${PROJECT_NAME}::Engine.Input.Schedule.Module
			${PROJECT_NAME}::Engine.Core.Schedule.Module ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Scene.Schedule.Module
)
# shortcut: ActorHousekeepingStage.cpp imports Engine.Scene.Schedule (ScenePipeline as stage-ordering anchor) and Engine.Scene.Schedule links ACS.Schedule, drop when pipeline ordering no longer names the other pipelines