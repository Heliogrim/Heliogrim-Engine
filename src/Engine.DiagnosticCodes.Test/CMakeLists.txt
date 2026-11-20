# Setup
make_test_project(
		TEST_NAME Engine.DiagnosticCodes.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Diagnostics
		${PROJECT_NAME}::Engine.Storage.Package
		${PROJECT_NAME}::Engine.Serialization
		${PROJECT_NAME}::Engine.Accel.Lang
		${PROJECT_NAME}::Engine.Async
		${PROJECT_NAME}::Engine.Config
		${PROJECT_NAME}::Engine.Core
		${PROJECT_NAME}::Engine.Event
		${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::Engine.Scheduler
		${PROJECT_NAME}::Engine.ACS
		${PROJECT_NAME}::Heliogrim
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Pedantic
		${PROJECT_NAME}::Engine.Reflect
)
