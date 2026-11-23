
# Setup
make_library_project(
		LIB_NAME Editor.Assets.Default
		LIB_FOLDER "${IDE_FOLDER}/Editor/Assets"
		LIBRARIES ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type
)
