
# Setup
make_library_project(
		LIB_NAME Editor.UI.Service
		MODULE
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES ${PROJECT_NAME}::Engine.Assets.System ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Assets.Database ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Graphics.Components ${PROJECT_NAME}::Engine.Physics.Components ${PROJECT_NAME}::Engine.Level ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Reflow ${PROJECT_NAME}::Engine.Reflow.Uikit ${PROJECT_NAME}::Editor.Action ${PROJECT_NAME}::Editor.Core ${PROJECT_NAME}::Editor.Scene.Actions
)
# shortcut: ContextService.cpp and PropertyServiceInit.cpp import Editor.UI (menu/input widgets), linked as Editor.UI.Module because Editor.UI links Editor.UI.Service, drop when those widgets move out of the services
target_link_libraries(${PROJECT_NAME}-Editor.UI.Service PRIVATE ${PROJECT_NAME}::Editor.UI.Module)
