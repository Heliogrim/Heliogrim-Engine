
# Setup
make_library_project(
		LIB_NAME Engine.Reflow.Uikit
		LIB_FOLDER "${IDE_FOLDER}/Engine/UI"
		LIBRARIES ${PROJECT_NAME}::Engine.Assets.System ${PROJECT_NAME}::Engine.Resource ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Input ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive
		${PROJECT_NAME}::Engine.Pedantic.Resource ${PROJECT_NAME}::Engine.Storage.Registry ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type
)