# Target Name
set(target Engine.Accel)

# Linker
set(link_libraries ${PROJECT_NAME}::Engine.Common)

# Setup
make_library_project(${target} "${link_libraries}")