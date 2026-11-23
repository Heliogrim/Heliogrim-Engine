
# Setup
make_library_project(
        LIB_NAME Engine.Physics.Scene.Model
		LIB_FOLDER "${IDE_FOLDER}/Engine/Physics"
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Physics ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Physics.Components ${PROJECT_NAME}::Engine.Scene.Support ${PROJECT_NAME}::Engine.Physics.Scene
)