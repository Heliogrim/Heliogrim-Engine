# Target Name
set(target Engine.Network.Test)

# Linker
set(link_libraries
        ${META_PROJECT_NAME}::Engine.Network
        ${META_PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")