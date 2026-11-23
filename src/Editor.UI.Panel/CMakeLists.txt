
# Setup
make_library_project(
		LIB_NAME Editor.UI.Panel
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Editor.Assets.Default ${PROJECT_NAME}::Engine.Assets.System ${PROJECT_NAME}::Engine.Resource ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Editor.UI.Rx ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Graphics.Scene ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Heliogrim.Actors ${PROJECT_NAME}::Engine.Physics.Components ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type
		${PROJECT_NAME}::Engine.Graphics.Core ${PROJECT_NAME}::Engine.Graphics.Render ${PROJECT_NAME}::Editor.UI.Theme ${PROJECT_NAME}::Engine.Reflow.Uikit
)
