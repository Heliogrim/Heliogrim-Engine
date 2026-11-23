# Setup
make_library_sub_project(
		LIB_NAME Engine.Reflow.Theming
		SUB_OF Engine.Reflow
		LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Resource.Archive
)
