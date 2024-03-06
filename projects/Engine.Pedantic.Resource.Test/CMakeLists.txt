
# Setup
make_test_project(
        TEST_NAME Engine.Pedantic.Resource.Test
        LIBRARIES
        ${PROJECT_NAME}::Engine.Asserts
        ${PROJECT_NAME}::Engine.Common
        ${PROJECT_NAME}::Engine.Pedantic.Resource
        ${PROJECT_NAME}::Engine.Scheduler
)