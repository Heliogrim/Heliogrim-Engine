
# Setup
make_library_project(
		LIB_NAME Engine.Graphics.Render.Subpass
		LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
		LIBRARIES
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Storage
		${PROJECT_NAME}::Engine.Storage.System
)