
# Setup
make_library_project(
		LIB_NAME Editor.Assets.Actions
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Config ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Logging
)
