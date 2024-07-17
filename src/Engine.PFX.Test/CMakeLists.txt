
# Setup
make_test_project(
        TEST_NAME Engine.PFX.Test
        LIBRARIES
        ${PROJECT_NAME}::Engine.Common
        ${PROJECT_NAME}::Engine.PFX
)