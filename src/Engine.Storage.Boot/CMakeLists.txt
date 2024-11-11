
# Setup
make_library_project(
		LIB_NAME Engine.Storage.Boot
		LIB_FOLDER "${IDE_FOLDER}/Engine/Storage"
		LIBRARIES ${PROJECT_NAME}::Engine.Common
)