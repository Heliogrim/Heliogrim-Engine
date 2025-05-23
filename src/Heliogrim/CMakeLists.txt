# Configure support libraries
set(__support_libraries "")
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
	list(
			APPEND __support_libraries
			${PROJECT_NAME}::Support.Input.Win32
			${PROJECT_NAME}::Support.Platform.Win32
	)
endif ()


# Setup
make_library_project(
		LIB_NAME Heliogrim
		LIB_FOLDER "${IDE_FOLDER}"
		FORCE_TYPE_STATIC ON
		LIBRARIES
		${PROJECT_NAME}::Engine.ACS
		${PROJECT_NAME}::Engine.ACS.Schedule
		${PROJECT_NAME}::Engine.Accel
		${PROJECT_NAME}::Engine.Accel.Command
		${PROJECT_NAME}::Engine.Accel.Compile
		${PROJECT_NAME}::Engine.Accel.Effect
		${PROJECT_NAME}::Engine.Accel.Lang
		${PROJECT_NAME}::Engine.Accel.Loader
		${PROJECT_NAME}::Engine.Accel.Pass
		${PROJECT_NAME}::Engine.Accel.Pipeline
		${PROJECT_NAME}::Engine.Accel.Storage
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Assets
		${PROJECT_NAME}::Engine.Assets.System
		${PROJECT_NAME}::Engine.Assets.Type
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Config
		${PROJECT_NAME}::Engine.Core
		${PROJECT_NAME}::Engine.Core.Schedule
		${PROJECT_NAME}::Engine.Crypto
		${PROJECT_NAME}::Engine.Driver.Vulkan
		${PROJECT_NAME}::Engine.Env
		${PROJECT_NAME}::Engine.Event
		${PROJECT_NAME}::Engine.Filesystem
		${PROJECT_NAME}::Engine.GFX
		${PROJECT_NAME}::Engine.GFX.Event
		${PROJECT_NAME}::Engine.GFX.Geometry
		${PROJECT_NAME}::Engine.GFX.Glow
		${PROJECT_NAME}::Engine.GFX.IO
		${PROJECT_NAME}::Engine.GFX.Loader
		${PROJECT_NAME}::Engine.GFX.Material
		${PROJECT_NAME}::Engine.GFX.Render
		${PROJECT_NAME}::Engine.GFX.Render.Command
		${PROJECT_NAME}::Engine.GFX.Render.Graph.Scene
		${PROJECT_NAME}::Engine.GFX.Render.Predefined
		${PROJECT_NAME}::Engine.GFX.Render.Subpass
		${PROJECT_NAME}::Engine.GFX.RenderGraph
		${PROJECT_NAME}::Engine.GFX.RenderGraph.Compile
		${PROJECT_NAME}::Engine.GFX.Scene
		${PROJECT_NAME}::Engine.GFX.Scene.Model
		${PROJECT_NAME}::Engine.GFX.Schedule
		${PROJECT_NAME}::Engine.Input
		${PROJECT_NAME}::Engine.Input.Schedule
		${PROJECT_NAME}::Engine.IO.Dispatcher
		${PROJECT_NAME}::Engine.Level
		${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::Engine.Network
		${PROJECT_NAME}::Engine.Pedantic
		${PROJECT_NAME}::Engine.Pedantic.Resource
		${PROJECT_NAME}::Engine.PFX
		${PROJECT_NAME}::Engine.Platform
		${PROJECT_NAME}::Engine.Reflect
		${PROJECT_NAME}::Engine.Reflow
		${PROJECT_NAME}::Engine.Reflow.Attributes
		${PROJECT_NAME}::Engine.Reflow.Render
		${PROJECT_NAME}::Engine.Reflow.Schedule
		${PROJECT_NAME}::Engine.Reflow.Theming
		${PROJECT_NAME}::Engine.Reflow.Uikit
		${PROJECT_NAME}::Engine.Render.Scene
		${PROJECT_NAME}::Engine.Render.Scene.Model
		${PROJECT_NAME}::Engine.Resource
		${PROJECT_NAME}::Engine.Resource.Archive
		${PROJECT_NAME}::Engine.Resource.Blob
		${PROJECT_NAME}::Engine.Resource.Package
		${PROJECT_NAME}::Engine.Scene
		${PROJECT_NAME}::Engine.Scene.Game
		${PROJECT_NAME}::Engine.Scene.Preview
		${PROJECT_NAME}::Engine.Scene.Schedule
		${PROJECT_NAME}::Engine.Scheduler
		${PROJECT_NAME}::Engine.Serialization
		${PROJECT_NAME}::Engine.Serialization.Layouts
		${PROJECT_NAME}::Engine.Serialization.Structures
		${PROJECT_NAME}::Engine.Storage
		${PROJECT_NAME}::Engine.Storage.Action
		${PROJECT_NAME}::Engine.Storage.Boot
		${PROJECT_NAME}::Engine.Storage.Package
		${PROJECT_NAME}::Engine.Storage.Registry
		${PROJECT_NAME}::Engine.Storage.System
		${PROJECT_NAME}::Engine.SFX
		${PROJECT_NAME}::Engine.Utils
		${__support_libraries}
)