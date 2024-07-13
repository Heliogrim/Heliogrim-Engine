
# Setup
make_test_project(
        TEST_NAME Engine.Resource.Archive.Test
        LIBRARIES
        ${PROJECT_NAME}::Engine.Asserts
        ${PROJECT_NAME}::Engine.Common
        ${PROJECT_NAME}::Engine.Filesystem
        ${PROJECT_NAME}::Engine.Resource
        ${PROJECT_NAME}::Engine.Resource.Archive
)