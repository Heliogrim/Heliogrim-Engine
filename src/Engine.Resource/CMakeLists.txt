
# Setup
make_library_project(
        LIB_NAME Engine.Resource
        LIB_FOLDER "${IDE_FOLDER}/Engine/IO"
        LIBRARIES ${PROJECT_NAME}::Engine.Serialization
        	${PROJECT_NAME}::Engine.Storage ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive
        ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Pedantic.Resource
        ${PROJECT_NAME}::Engine.IO.Dispatcher
)