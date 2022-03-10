# Target Name
set(target Ember.Main)

# Linker
set(
        link_libraries
        ${META_PROJECT_NAME}::Engine.ACS
        ${META_PROJECT_NAME}::Engine.Assets
        ${META_PROJECT_NAME}::Engine.Common
        ${META_PROJECT_NAME}::Engine.Core
        ${META_PROJECT_NAME}::Engine.Env
        ${META_PROJECT_NAME}::Engine.Event
        ${META_PROJECT_NAME}::Engine.GFX
        ${META_PROJECT_NAME}::Engine.Network
        ${META_PROJECT_NAME}::Engine.PFX
        ${META_PROJECT_NAME}::Engine.Reflect
        ${META_PROJECT_NAME}::Engine.Resource
        ${META_PROJECT_NAME}::Engine.Scene
        ${META_PROJECT_NAME}::Engine.Scheduler
        ${META_PROJECT_NAME}::Engine.Session
        ${META_PROJECT_NAME}::Engine.SFX
        ${META_PROJECT_NAME}::Engine.Utils
)

# Setup
make_library_project(${target} "${link_libraries}")