
# Setup
make_library_project(
        LIB_NAME "Editor.Action"
        LIB_FOLDER "${IDE_FOLDER}/Editor"
        LIBRARIES
        	${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Storage ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive
)
