
# Setup
make_library_project(
		LIB_NAME Editor.Scene.Actions
		MODULE
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES
			${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Assets.Database ${PROJECT_NAME}::Engine.Assets.System ${PROJECT_NAME}::Editor.Assets.Default ${PROJECT_NAME}::Engine.Storage ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Editor.UI.Rx ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Heliogrim.Actors ${PROJECT_NAME}::Engine.Graphics.Components ${PROJECT_NAME}::Engine.Level ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Storage.Action ${PROJECT_NAME}::Engine.Storage.System ${PROJECT_NAME}::Editor.Action ${PROJECT_NAME}::Editor.Core
)
# shortcut: the Spawn*Actor.cpp actions import Editor.UI.Main (editor UI model) and Editor.UI.Main links Editor.UI.Service, which links Editor.Scene.Actions, drop when the spawn actions get the scene through an action context
target_link_libraries(${PROJECT_NAME}-Editor.Scene.Actions PRIVATE ${PROJECT_NAME}::Editor.UI.Main.Module)
