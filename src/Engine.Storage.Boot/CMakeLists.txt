
# Setup
make_library_project(
		LIB_NAME Engine.Storage.Boot
		LIB_FOLDER "${IDE_FOLDER}/Engine/Storage"
		LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Reflect.Utils ${PROJECT_NAME}::Engine.Core
)