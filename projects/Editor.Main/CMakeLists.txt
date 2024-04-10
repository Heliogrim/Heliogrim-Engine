
# Setup
make_app_project(
		APP_NAME "Editor"
		LIB_NAME "Editor.Main"
		SOURCE "Editor.Main"
		LIBRARIES
		${PROJECT_NAME}::Editor.Action
		${PROJECT_NAME}::Editor.Core
		${PROJECT_NAME}::Editor.GFX.Event
		${PROJECT_NAME}::Editor.GFX.Graphs
		${PROJECT_NAME}::Editor.GFX.IO
		${PROJECT_NAME}::Editor.GFX.Loader
		${PROJECT_NAME}::Editor.Render
		${PROJECT_NAME}::Editor.Scene
		${PROJECT_NAME}::Editor.UI
		${PROJECT_NAME}::Heliogrim
		${PROJECT_NAME}::Heliogrim.Default
)