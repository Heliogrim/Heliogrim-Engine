
# Setup
make_library_project(
        LIB_NAME Engine.Assets.System
        LIBRARIES ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Resource.Archive
)