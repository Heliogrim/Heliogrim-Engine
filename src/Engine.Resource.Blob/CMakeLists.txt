
# Setup
make_library_project(
		LIB_NAME Engine.Resource.Blob
		LIB_FOLDER "${IDE_FOLDER}/Engine/IO"
		LIBRARIES
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Async
		${PROJECT_NAME}::Engine.IO.Dispatcher
)