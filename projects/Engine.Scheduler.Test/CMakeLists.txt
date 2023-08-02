# Target Name
set(target Engine.Scheduler.Test)

# Linker
set(link_libraries
        ${PROJECT_NAME}::Engine.Scheduler
        ${PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")