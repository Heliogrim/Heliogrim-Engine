
# Setup
make_library_project(
		LIB_NAME Editor.UI.Theme
		MODULE
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Editor.UI.Rx ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Reflow
)
# shortcut: EditorTheme.cpp imports Editor.UI.Main (theme lookup through the editor UI) and Editor.UI.Main links Editor.UI.Theme, drop when the theme is handed in instead of looked up
target_link_libraries(${PROJECT_NAME}-Editor.UI.Theme PRIVATE ${PROJECT_NAME}::Editor.UI.Main.Module)
