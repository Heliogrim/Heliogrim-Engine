# Target Name
set(target Heliogrim.Main)

# Linker
set(
        link_libraries
	${PROJECT_NAME}::Heliogrim
	${PROJECT_NAME}::Engine.Core.Game
)

# Setup
make_library_project(${target} "${link_libraries}")
