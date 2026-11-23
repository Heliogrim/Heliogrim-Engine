
# Setup
make_library_project(
        LIB_NAME Engine.Accel.Storage
        LIBRARIES ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Accel.Lang ${PROJECT_NAME}::Engine.Accel.Effect ${PROJECT_NAME}::Engine.Graphics.RenderGraph ${PROJECT_NAME}::Engine.Accel.Pipeline
)