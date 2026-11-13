
# Setup
make_library_project(
		LIB_NAME Engine.Storage.Action
		LIB_FOLDER "${IDE_FOLDER}/Engine/Storage"
		LIBRARIES ${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Async
		${PROJECT_NAME}::Engine.IO.Dispatcher
)