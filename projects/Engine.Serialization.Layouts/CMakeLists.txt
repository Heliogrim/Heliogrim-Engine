# Target Name
set(target Engine.Serialization.Layouts)

# Linker
set(link_libraries
        ${PROJECT_NAME}::Engine.Common
        ${PROJECT_NAME}::Engine.Scheduler)

# Setup
make_library_project(${target} "${link_libraries}")