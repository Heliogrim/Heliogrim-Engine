# Setup
make_library_sub_project(
		LIB_NAME Engine.Reflow.Attributes
		SUB_OF Engine.Reflow
		LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Assets
)
