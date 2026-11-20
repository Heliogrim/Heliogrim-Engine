
# Setup
make_library_project(
		LIB_NAME Engine.Reflow.Uikit
		LIB_FOLDER "${IDE_FOLDER}/Engine/UI"
		LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive
)