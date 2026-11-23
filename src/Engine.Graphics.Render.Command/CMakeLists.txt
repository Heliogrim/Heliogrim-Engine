
# Setup
make_library_project(
        LIB_NAME Engine.Graphics.Render.Command
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Resource ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Graphics.Types ${PROJECT_NAME}::Engine.Storage.Registry ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Accel.Pass
)