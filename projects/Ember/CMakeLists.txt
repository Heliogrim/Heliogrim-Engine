# Target Name
set(target Ember)

# Linker
set(
        link_libraries
		${META_PROJECT_NAME}::Engine.ACS
		${META_PROJECT_NAME}::Engine.Assets
		${META_PROJECT_NAME}::Engine.Common
		${META_PROJECT_NAME}::Engine.Core
		${META_PROJECT_NAME}::Engine.Core.Schedule
		${META_PROJECT_NAME}::Engine.Env
		${META_PROJECT_NAME}::Engine.Event
		${META_PROJECT_NAME}::Engine.GFX
		${META_PROJECT_NAME}::Engine.GFX.Glow
		${META_PROJECT_NAME}::Engine.GFX.Glow.3D
		${META_PROJECT_NAME}::Engine.GFX.Glow.UI
		${META_PROJECT_NAME}::Engine.GFX.Loader
		${META_PROJECT_NAME}::Engine.GFX.Scene
		${META_PROJECT_NAME}::Engine.GFX.Schedule
		${META_PROJECT_NAME}::Engine.Input
		${META_PROJECT_NAME}::Engine.Input.Schedule
		${META_PROJECT_NAME}::Engine.Logging
		${META_PROJECT_NAME}::Engine.Network
		${META_PROJECT_NAME}::Engine.PFX
		${META_PROJECT_NAME}::Engine.Platform
		${META_PROJECT_NAME}::Engine.Reflect
		${META_PROJECT_NAME}::Engine.Reflow
		${META_PROJECT_NAME}::Engine.Reflow.Schedule
		${META_PROJECT_NAME}::Engine.Resource
		${META_PROJECT_NAME}::Engine.Resource.Package
		${META_PROJECT_NAME}::Engine.Scene
		${META_PROJECT_NAME}::Engine.Scheduler
		${META_PROJECT_NAME}::Engine.Serialization
		${META_PROJECT_NAME}::Engine.Serialization.Layouts
		${META_PROJECT_NAME}::Engine.Serialization.Structures
		${META_PROJECT_NAME}::Engine.SFX
		${META_PROJECT_NAME}::Engine.Utils
)

# Setup
make_library_project(${target} "${link_libraries}")