
# Setup
make_test_project(
        TEST_NAME Engine.Storage.Test
        LIBRARIES
        ${PROJECT_NAME}::Engine.Common
        ${PROJECT_NAME}::Engine.Asserts
        ${PROJECT_NAME}::Engine.Config
        ${PROJECT_NAME}::Engine.Filesystem
        ${PROJECT_NAME}::Engine.Reflect
        ${PROJECT_NAME}::Engine.Storage
)