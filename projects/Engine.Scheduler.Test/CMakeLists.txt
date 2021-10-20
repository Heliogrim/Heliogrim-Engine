# Target Name
set(target Engine.Scheduler.Test)

# Linker
set(link_libraries
        ${META_PROJECT_NAME}::Engine.Scheduler
        ${META_PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")