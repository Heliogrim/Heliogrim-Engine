
# Setup
make_library_project(
		LIB_NAME "Editor.Boot"
		MODULE
		LIB_FOLDER "${IDE_FOLDER}/Editor"
		LIBRARIES
			${PROJECT_NAME}::Engine.Level ${PROJECT_NAME}::Engine.Resource.Package ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Reflow
			${PROJECT_NAME}::Engine.Storage
		${PROJECT_NAME}::Engine.Serialization
		${PROJECT_NAME}::Engine.Serialization.Structures
		${PROJECT_NAME}::Engine.Assets.Database ${PROJECT_NAME}::Engine.Assets.System ${PROJECT_NAME}::Engine.Resource
		${PROJECT_NAME}::Editor.Action
		${PROJECT_NAME}::Editor.Assets.Actions
		${PROJECT_NAME}::Editor.Assets.Default
		${PROJECT_NAME}::Editor.Core
		${PROJECT_NAME}::Editor.Graphics.Event
		${PROJECT_NAME}::Editor.Graphics.Graphs
		${PROJECT_NAME}::Editor.Graphics.IO
		${PROJECT_NAME}::Editor.Graphics.Loader
		${PROJECT_NAME}::Editor.Render
		${PROJECT_NAME}::Editor.Scene
		${PROJECT_NAME}::Editor.Scene.Actions
		${PROJECT_NAME}::Editor.UI
		${PROJECT_NAME}::Editor.UI.Init
		${PROJECT_NAME}::Editor.UI.Main
		${PROJECT_NAME}::Editor.UI.Panel
		${PROJECT_NAME}::Editor.UI.Rx
		${PROJECT_NAME}::Editor.UI.Service
		${PROJECT_NAME}::Editor.UI.Theme
		${PROJECT_NAME}::Editor.UI.View
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Config
		${PROJECT_NAME}::Engine.Config.Providers
		${PROJECT_NAME}::Engine.Core
		${PROJECT_NAME}::Engine.Logging
		${PROJECT_NAME}::Engine.Reflect
		${PROJECT_NAME}::Engine.Resource.Archive
		${PROJECT_NAME}::Heliogrim
		${PROJECT_NAME}::Heliogrim.Actors
		${PROJECT_NAME}::Engine.Graphics.Components
		${PROJECT_NAME}::Engine.Async
		${PROJECT_NAME}::Engine.Scheduler
		${PROJECT_NAME}::Engine.Storage.Registry
		${PROJECT_NAME}::Engine.Assets
		${PROJECT_NAME}::Engine.Storage.Action
		${PROJECT_NAME}::Engine.Storage.Package
		${PROJECT_NAME}::Engine.Assets.Type
		${PROJECT_NAME}::Engine.Storage.System
		${PROJECT_NAME}::Engine.Graphics.Core
)