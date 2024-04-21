
# Setup
make_test_project(
        TEST_NAME Engine.Network.Test
        LIBRARIES
        ${PROJECT_NAME}::Engine.Common
        ${PROJECT_NAME}::Engine.Network
)