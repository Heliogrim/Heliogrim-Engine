
# Setup
make_test_project(
		TEST_NAME Engine.Storage.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.ACS
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Config
		${PROJECT_NAME}::Engine.Core
		${PROJECT_NAME}::Engine.Filesystem
		${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::Engine.Reflect
		${PROJECT_NAME}::Engine.Resource.Archive
		${PROJECT_NAME}::Engine.Resource.Blob
		${PROJECT_NAME}::Engine.Resource.Package
		${PROJECT_NAME}::Engine.Scheduler
		${PROJECT_NAME}::Engine.Serialization
		${PROJECT_NAME}::Engine.Serialization.Layouts
		${PROJECT_NAME}::Engine.Serialization.Structures
		${PROJECT_NAME}::Engine.Storage
		${PROJECT_NAME}::Engine.Storage.Action
		${PROJECT_NAME}::Engine.Storage.Boot
		${PROJECT_NAME}::Engine.Storage.Package
		${PROJECT_NAME}::Engine.Storage.Registry
		${PROJECT_NAME}::Engine.Storage.System
		${PROJECT_NAME}::Heliogrim
)