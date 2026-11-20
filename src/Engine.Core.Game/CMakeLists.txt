
# Setup
make_library_project(
        LIB_NAME Engine.Core.Game
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Config ${PROJECT_NAME}::Engine.Core
)