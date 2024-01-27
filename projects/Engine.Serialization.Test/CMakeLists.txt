# Target Name
set(target Engine.Serialization.Test)

# Linker
set(link_libraries
	${PROJECT_NAME}::Engine.Assets
	${PROJECT_NAME}::Engine.Common
	${PROJECT_NAME}::Engine.Filesystem
	${PROJECT_NAME}::Engine.Logging
	${PROJECT_NAME}::Engine.Reflect
	${PROJECT_NAME}::Engine.Serialization
	${PROJECT_NAME}::Engine.Serialization.Layouts
	${PROJECT_NAME}::Engine.Serialization.Structures
)

# Setup
make_test_project(${target} "${link_libraries}")