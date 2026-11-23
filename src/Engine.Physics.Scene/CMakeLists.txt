
# Setup
make_library_project(
        LIB_NAME Engine.Physics.Scene
		LIB_FOLDER "${IDE_FOLDER}/Engine/Physics"
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Physics ${PROJECT_NAME}::Engine.Physics.Components
)