
# Setup
make_library_project(
        LIB_NAME Engine.Core
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/Core"
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Config ${PROJECT_NAME}::Engine.Event ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive
)
# shortcut: Universe.cpp imports Engine.Level and Engine.Scene (both sit above Core) and so also Engine.Async, Engine.Serialization and Engine.ACS; SessionState.cpp imports Engine.ACS directly (the session owns the ACS registry); drop when Universe stops using Level/Scene and Core sessions no longer own the ACS registry (service-locator design)
target_link_libraries(${PROJECT_NAME}-Engine.Core PRIVATE ${PROJECT_NAME}::Engine.Async.Module ${PROJECT_NAME}::Engine.Serialization.Module ${PROJECT_NAME}::Engine.ACS.Module ${PROJECT_NAME}::Engine.Level.Module ${PROJECT_NAME}::Engine.Scene.Module)