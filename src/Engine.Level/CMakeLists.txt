
# Setup
make_library_project(
		LIB_NAME Engine.Level
		MODULE
		LIB_FOLDER "${IDE_FOLDER}/Engine/Level"
		LIBRARIES ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Serialization.Structures ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Scene ${PROJECT_NAME}::Engine.Storage.Registry ${PROJECT_NAME}::Engine.Assets
)
# shortcut: Level.cpp imports Engine.Assets.Type (LevelAsset) and Engine.Assets.Type links Engine.Level, drop when the level loader moves out of Engine.Level
target_link_libraries(${PROJECT_NAME}-Engine.Level PRIVATE ${PROJECT_NAME}::Engine.Assets.Type.Module)
