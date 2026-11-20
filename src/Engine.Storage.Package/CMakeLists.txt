
# Setup
make_library_project(
		LIB_NAME Engine.Storage.Package
		LIB_FOLDER "${IDE_FOLDER}/Engine/Storage"
		LIBRARIES
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Crypto
		${PROJECT_NAME}::Engine.Diagnostics
		${PROJECT_NAME}::Engine.Asserts
)
