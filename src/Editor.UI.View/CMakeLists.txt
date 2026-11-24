
# Setup
make_library_project(
		LIB_NAME Editor.UI.View
		MODULE
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Editor.UI.Rx ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Reflow ${PROJECT_NAME}::Engine.Reflow.Uikit ${PROJECT_NAME}::Editor.UI
)
# shortcut: SceneEditorController.cpp and SceneEditorView.cpp import Editor.UI.Panel (panel controllers/views), linked as Editor.UI.Panel.Module because Editor.UI.Panel links Editor.UI.Main which links Editor.UI.View, drop when the scene editor view moves into Editor.UI.Panel
target_link_libraries(${PROJECT_NAME}-Editor.UI.View PRIVATE ${PROJECT_NAME}::Editor.UI.Panel.Module)
# shortcut: the Editor.UI.Panel interface re-exports Editor.UI.Main (EditorUiModel), linked as Editor.UI.Main.Module because Editor.UI.Main links Editor.UI.View, drop with the Editor.UI.Panel.Module shortcut above
target_link_libraries(${PROJECT_NAME}-Editor.UI.View PRIVATE ${PROJECT_NAME}::Editor.UI.Main.Module)
