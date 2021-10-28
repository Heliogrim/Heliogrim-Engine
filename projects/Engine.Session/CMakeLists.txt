# Target Name
set(target Engine.Session)

# Linker
set(link_libraries
        ${META_PROJECT_NAME}::Engine.Common
        ${META_PROJECT_NAME}::Engine.Scheduler)

# Setup
make_library_project(${target} "${link_libraries}")