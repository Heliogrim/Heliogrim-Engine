
# Setup
make_library_project(
        LIB_NAME Engine.Accel.Compile
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::vulkan.Module
)