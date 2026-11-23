
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.IO
        LIB_FOLDER "${IDE_FOLDER}/Engine/IO"
        LIBRARIES ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Graphics.Event ${PROJECT_NAME}::Engine.Resource.Archive
        ${PROJECT_NAME}::Engine.Pedantic.Resource
)