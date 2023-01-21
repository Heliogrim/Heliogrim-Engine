# Target Name
set(target Engine.Resource.Package.Test)

# Linker
set(link_libraries
        ${META_PROJECT_NAME}::Engine.Resource
	${META_PROJECT_NAME}::Engine.Resource.Package
	${META_PROJECT_NAME}::Engine.Serialization
        ${META_PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")