
# Setup
make_library_project(
		LIB_NAME Editor.UI.View
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Editor.UI.Rx ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Reflow
)
