
# Setup
make_test_project(
        TEST_NAME Engine.Scheduler.Test
        LIBRARIES
        ${PROJECT_NAME}::Engine.Common
        ${PROJECT_NAME}::Engine.Core
        ${PROJECT_NAME}::Engine.Event
        ${PROJECT_NAME}::Engine.Logging
        ${PROJECT_NAME}::Engine.Scheduler
)