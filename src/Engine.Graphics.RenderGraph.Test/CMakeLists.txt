# Setup
make_test_project(
		TEST_NAME Engine.Graphics.RenderGraph.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Graphics
		${PROJECT_NAME}::Engine.Graphics.RenderGraph
		${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::Engine.Async
		${PROJECT_NAME}::Engine.Resource
		${PROJECT_NAME}::Engine.Scheduler
		${PROJECT_NAME}::Heliogrim
		${PROJECT_NAME}::Engine.Pedantic
		${PROJECT_NAME}::Engine.Reflect
)
