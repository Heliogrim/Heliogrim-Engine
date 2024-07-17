
# Setup
make_test_project(
        TEST_NAME Engine.Event.Test
        LIBRARIES
        ${PROJECT_NAME}::Engine.Event
        ${PROJECT_NAME}::Engine.Common
)