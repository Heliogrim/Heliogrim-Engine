# Target Name
set(target Engine.Scene.Test)

# Linker
set(link_libraries
        ${META_PROJECT_NAME}::Engine.Scene
        ${META_PROJECT_NAME}::Engine.Common
        ${META_PROJECT_NAME}::Engine.Event
        ${META_PROJECT_NAME}::Engine.Scheduler)

# Setup
make_test_project(${target} "${link_libraries}")