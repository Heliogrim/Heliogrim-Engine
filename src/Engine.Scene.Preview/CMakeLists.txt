
# Setup
make_library_project(
        LIB_NAME Engine.Scene.Preview
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Scene ${PROJECT_NAME}::Engine.ACS
)