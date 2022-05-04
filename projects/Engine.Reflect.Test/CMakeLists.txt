# Target Name
set(target Engine.Reflect.Test)

# Linker
set(link_libraries
        ${META_PROJECT_NAME}::Engine.Reflect
        ${META_PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")