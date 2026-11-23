
# Setup
make_library_project(
        LIB_NAME Engine.Physics.Scene
        MODULE
		LIB_FOLDER "${IDE_FOLDER}/Engine/Physics"
        LIBRARIES ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Physics ${PROJECT_NAME}::Engine.Physics.Components ${PROJECT_NAME}::Engine.Scene ${PROJECT_NAME}::Engine.Scene.Support ${PROJECT_NAME}::Engine.Physics.Scene.Model.Module
)
# shortcut: PhysicsSceneSystem.cpp uses Engine.Physics.Scene.Model (collision component-to-model type mappings), drop when model mappings are registered by Engine.Physics.Scene.Model