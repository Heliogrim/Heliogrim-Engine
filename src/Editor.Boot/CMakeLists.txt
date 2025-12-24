
# Setup
make_library_project(
		LIB_NAME "Editor.Boot"
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES
		${PROJECT_NAME}::Editor.Action
		${PROJECT_NAME}::Editor.Assets.Actions
		${PROJECT_NAME}::Editor.Assets.Default
		${PROJECT_NAME}::Editor.Core
		${PROJECT_NAME}::Editor.GFX.Event
		${PROJECT_NAME}::Editor.GFX.Graphs
		${PROJECT_NAME}::Editor.GFX.IO
		${PROJECT_NAME}::Editor.GFX.Loader
		${PROJECT_NAME}::Editor.Render
		${PROJECT_NAME}::Editor.Scene
		${PROJECT_NAME}::Editor.Scene.Actions
		${PROJECT_NAME}::Editor.UI
		${PROJECT_NAME}::Editor.UI.Init
		${PROJECT_NAME}::Editor.UI.Main
		${PROJECT_NAME}::Editor.UI.Panel
		${PROJECT_NAME}::Editor.UI.Rx
		${PROJECT_NAME}::Editor.UI.Service
		${PROJECT_NAME}::Editor.UI.Theme
		${PROJECT_NAME}::Editor.UI.View
		${PROJECT_NAME}::Heliogrim
)