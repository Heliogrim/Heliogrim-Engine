
# Setup
make_library_project(
		LIB_NAME Editor.Assets.Default
		MODULE
		LIB_FOLDER "${IDE_FOLDER}/Editor/Assets"
		LIBRARIES ${PROJECT_NAME}::Engine.Assets.System ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Graphics.Render.Predefined
)
