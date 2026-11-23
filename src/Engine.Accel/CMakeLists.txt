
# Setup
make_library_project(
        LIB_NAME Engine.Accel
        LIB_FOLDER "${IDE_FOLDER}/Engine/Accel"
        LIBRARIES ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Resource.Archive
)