# Target Name
set(target Engine.Assets.Test)

# Linker
set(link_libraries
        ${META_PROJECT_NAME}::Engine.Assets
        ${META_PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")