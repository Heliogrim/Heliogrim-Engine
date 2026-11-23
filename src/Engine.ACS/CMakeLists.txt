
# Setup
make_library_project(
        LIB_NAME Engine.ACS
        LIBRARIES ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Serialization.Structures
)
# shortcut: Actor/Actor.cpp textually includes Engine.Level (Level), Engine.Scene (SceneBase, IComponentRegisterContext) and Heliogrim (Session, Universe), resolved through Heliogrim's link of the executables; drop when actor spawning moves above ACS