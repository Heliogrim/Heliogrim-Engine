
# Setup
make_library_project(
		LIB_NAME Engine.Storage.System
		LIB_FOLDER "${IDE_FOLDER}/Engine/Storage"
		LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Reflect.Utils
)