# Target Name
set(target Engine.Resource.Package.Test)

# Linker
set(link_libraries
	${PROJECT_NAME}::Engine.Common
	${PROJECT_NAME}::Engine.Filesystem
	${PROJECT_NAME}::Engine.Resource
	${PROJECT_NAME}::Engine.Resource.Package
	${PROJECT_NAME}::Engine.Serialization
	${PROJECT_NAME}::Engine.Serialization.Layouts
	${PROJECT_NAME}::Engine.Serialization.Structures
)

# Setup
make_test_project(${target} "${link_libraries}")