# Target Name
set(target Engine.Network.Test)

# Linker
set(link_libraries
        ${PROJECT_NAME}::Engine.Network
        ${PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")