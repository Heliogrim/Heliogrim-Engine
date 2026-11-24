
# Setup
make_library_project(
        LIB_NAME Engine.Reflow.Schedule
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/UI"
        LIBRARIES ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Input
        	${PROJECT_NAME}::Engine.Graphics.Schedule ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive
        ${PROJECT_NAME}::Engine.Platform ${PROJECT_NAME}::Engine.Scene.Schedule ${PROJECT_NAME}::Engine.Storage.Registry ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Reflow.Module
)
# shortcut: ReflowFlowStage.cpp imports Engine.Reflow (window manager tick), linked as Engine.Reflow.Module because Engine.Reflow links Engine.Reflow.Schedule, Reflow is a sub-module and has no module slot, drop when Engine.Reflow registers the stage itself