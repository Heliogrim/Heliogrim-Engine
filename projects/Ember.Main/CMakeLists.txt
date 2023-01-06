# Target Name
set(target Ember.Main)

# Linker
set(
        link_libraries
        ${META_PROJECT_NAME}::Engine.Common
        ${META_PROJECT_NAME}::Engine.Core
        ${META_PROJECT_NAME}::Engine.Core.Game
)

# Setup
make_library_project(${target} "${link_libraries}")