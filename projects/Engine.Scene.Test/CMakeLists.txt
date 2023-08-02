# Target Name
set(target Engine.Scene.Test)

# Linker
set(link_libraries
        ${PROJECT_NAME}::Engine.Scene
        ${PROJECT_NAME}::Engine.Common
        ${PROJECT_NAME}::Engine.Event
        ${PROJECT_NAME}::Engine.Scheduler)

# Setup
make_test_project(${target} "${link_libraries}")