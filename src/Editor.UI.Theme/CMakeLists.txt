
# Setup
make_library_project(
		LIB_NAME Editor.UI.Theme
		MODULE
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Editor.UI.Rx ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Reflow
)
