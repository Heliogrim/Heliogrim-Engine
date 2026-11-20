# Setup
make_test_project(
		TEST_NAME Heliogrim.Test
		LIBRARIES
		${PROJECT_NAME}::Engine.ACS
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Config
		${PROJECT_NAME}::Engine.Core
		${PROJECT_NAME}::Engine.Event
		${PROJECT_NAME}::Engine.Reflect
		${PROJECT_NAME}::Engine.Resource.Archive
		${PROJECT_NAME}::Heliogrim
)

name_to_target(__target Heliogrim.Test)
target_compile_definitions(${__target} PRIVATE HG_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/serialization")
