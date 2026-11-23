
# Setup
make_library_project(
		LIB_NAME Engine.Scene.Support
		LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.ACS
)