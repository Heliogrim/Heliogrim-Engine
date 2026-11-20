
# Setup
make_library_project(
		LIB_NAME "Editor.Graphics.Graphs"
		MODULE
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES ${PROJECT_NAME}::Heliogrim ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Common
		MODULE_EXCLUDE
		# Unused test scaffolds; their inline builders call members that have no definition.
		Editor.Graphics.Graphs/__TestGraphs.hpp
		Editor.Graphics.Graphs/__TestLang.hpp
)
