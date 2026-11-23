
# Setup
make_library_project(
		LIB_NAME Editor.UI.Service
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Assets.Database ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core
)
