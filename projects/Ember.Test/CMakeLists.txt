# Target Name
set(target Ember.Test)

# Linker
set(link_libraries
        ${META_PROJECT_NAME}::Ember
        ${META_PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")