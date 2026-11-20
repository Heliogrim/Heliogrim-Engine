
# Setup
make_library_project(
        LIB_NAME Engine.Core.Game
        LIBRARIES ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Config ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Network ${PROJECT_NAME}::Engine.Physics ${PROJECT_NAME}::Engine.Platform
)