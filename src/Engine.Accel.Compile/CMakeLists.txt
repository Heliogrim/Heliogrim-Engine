
# Setup
make_library_project(
        LIB_NAME Engine.Accel.Compile
        LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Env ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Graphics.Types
        ${PROJECT_NAME}::Engine.Graphics.Core ${PROJECT_NAME}::Engine.Accel.Lang ${PROJECT_NAME}::Engine.Accel.Effect ${PROJECT_NAME}::Engine.Accel.Pass ${PROJECT_NAME}::Engine.Graphics.RenderGraph
)