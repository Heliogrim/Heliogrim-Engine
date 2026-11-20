
# Setup
make_library_project(
        LIB_NAME Engine.IO.Dispatcher
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::Engine.Async
)