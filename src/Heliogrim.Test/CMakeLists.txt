# Setup
make_test_project(
		TEST_NAME Heliogrim.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.Level
		${PROJECT_NAME}::Engine.ACS
		${PROJECT_NAME}::Engine.Serialization.Structures
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Config
		${PROJECT_NAME}::Engine.Serialization
		${PROJECT_NAME}::Engine.Core
		${PROJECT_NAME}::Engine.Event
		${PROJECT_NAME}::Engine.Reflect
		${PROJECT_NAME}::Engine.Resource.Archive
		${PROJECT_NAME}::Engine.Scene
		${PROJECT_NAME}::Engine.Scene.Game
		${PROJECT_NAME}::Heliogrim
		${PROJECT_NAME}::Heliogrim.Actors
		${PROJECT_NAME}::Engine.Graphics.Components
		${PROJECT_NAME}::Engine.Physics.Components
		${PROJECT_NAME}::Engine.Assets
)

name_to_target(__target Heliogrim.Test)
target_compile_definitions(${__target} PRIVATE HG_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/serialization")
