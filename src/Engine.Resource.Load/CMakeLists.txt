
# Setup
make_library_project(
		LIB_NAME Engine.Resource.Load
		MODULE
		LIB_FOLDER "${IDE_FOLDER}/Engine/IO"
		LIBRARIES ${PROJECT_NAME}::Engine.Common
)