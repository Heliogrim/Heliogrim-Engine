# Setup
make_library_project(
        LIB_NAME Engine.Physics.Components
        LIB_FOLDER "${IDE_FOLDER}/Engine/Physics"
        LIBRARIES ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect
)
