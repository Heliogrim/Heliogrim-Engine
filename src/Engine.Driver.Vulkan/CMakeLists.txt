
# Setup
make_library_project(
        LIB_NAME Engine.Driver.Vulkan
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Driver"
        LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Graphics.Types
        ${PROJECT_NAME}::Engine.Graphics.Core ${PROJECT_NAME}::Engine.Graphics.RenderGraph ${PROJECT_NAME}::Engine.Accel.Pass ${PROJECT_NAME}::Engine.Accel.Pipeline ${PROJECT_NAME}::Engine.Accel.Command ${PROJECT_NAME}::Engine.Graphics.Render.Command
)