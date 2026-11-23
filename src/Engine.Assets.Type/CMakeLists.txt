
# Setup
make_library_project(
		LIB_NAME Engine.Assets.Type
		LIBRARIES ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Serialization.Structures ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Level ${PROJECT_NAME}::Engine.Graphics.Types
)