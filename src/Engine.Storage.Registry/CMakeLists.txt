
# Setup
make_library_project(
		LIB_NAME Engine.Storage.Registry
		MODULE
		LIB_FOLDER "${IDE_FOLDER}/Engine/Storage"
		LIBRARIES ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Env ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Config ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive
)