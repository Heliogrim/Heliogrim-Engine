
# Setup
make_library_project(
		LIB_NAME Engine.Storage.Action
		LIB_FOLDER "${IDE_FOLDER}/Engine/Storage"
		LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Reflect.Utils
		${PROJECT_NAME}::Engine.Async
		${PROJECT_NAME}::Engine.IO.Dispatcher ${PROJECT_NAME}::Engine.Pedantic.Resource ${PROJECT_NAME}::Engine.Storage.Registry
)