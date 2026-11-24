
# Setup
make_library_project(
		LIB_NAME Editor.Scene.Actions
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES
			${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Assets.Database ${PROJECT_NAME}::Engine.Assets.System ${PROJECT_NAME}::Editor.Assets.Default ${PROJECT_NAME}::Engine.Storage ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Editor.UI.Rx ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Heliogrim.Actors ${PROJECT_NAME}::Engine.Graphics.Components ${PROJECT_NAME}::Engine.Level ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Storage.Action ${PROJECT_NAME}::Engine.Storage.System ${PROJECT_NAME}::Editor.Action ${PROJECT_NAME}::Editor.Core
)
