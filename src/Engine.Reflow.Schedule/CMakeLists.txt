
# Setup
make_library_project(
        LIB_NAME Engine.Reflow.Schedule
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/UI"
        LIBRARIES ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Input
        	${PROJECT_NAME}::Engine.Graphics.Schedule ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive
        ${PROJECT_NAME}::Engine.Platform ${PROJECT_NAME}::Engine.Scene.Schedule
)