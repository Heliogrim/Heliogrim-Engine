
# Setup
make_library_project(
        LIB_NAME Engine.Serialization.Structures
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/IO"
        LIBRARIES ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Resource.Archive
)