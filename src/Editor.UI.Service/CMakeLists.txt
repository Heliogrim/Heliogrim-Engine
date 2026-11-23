
# Setup
make_library_project(
		LIB_NAME Editor.UI.Service
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES ${PROJECT_NAME}::Engine.Assets.System ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Assets.Database ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Graphics.Components ${PROJECT_NAME}::Engine.Physics.Components ${PROJECT_NAME}::Engine.Level ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type
)
