
# Setup
make_library_project(
		LIB_NAME Engine.Storage.System
		LIB_FOLDER "${IDE_FOLDER}/Engine/Storage"
		LIBRARIES ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Reflect.Utils
		${PROJECT_NAME}::Engine.Pedantic.Resource ${PROJECT_NAME}::Engine.Resource.Blob ${PROJECT_NAME}::Engine.Resource.Package ${PROJECT_NAME}::Engine.Storage.Registry ${PROJECT_NAME}::Engine.Storage.Action ${PROJECT_NAME}::Engine.Storage.Package
)