
# Setup
make_library_project(
        LIB_NAME Engine.Core.Schedule
        MODULE
        LIBRARIES
        	${PROJECT_NAME}::Engine.ACS.Schedule.Module ${PROJECT_NAME}::Engine.Scene.Schedule.Module ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core
)
# shortcut: CorePipeline.cpp imports Engine.Scene.Schedule (ScenePipeline as stage-ordering anchor) and Engine.Scene.Schedule links Core.Schedule, drop when pipeline ordering no longer names the other pipelines