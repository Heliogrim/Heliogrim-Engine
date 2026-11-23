
# Setup
make_library_project(
        LIB_NAME Engine.ACS
        MODULE
        MODULE_EXCLUDE
        # Unused and stale: its view template names Pool with the old one-argument signature.
        Engine.ACS/View.hpp
        # Definition headers for archetype-defining TUs only (STYLE.md); importers must not instantiate them.
        Engine.ACS/Archetype/ArchetypeDefinition.inl.hpp
        Engine.ACS/Archetype/ArchetypeLayout.inl.hpp
        Engine.ACS/Archetype/ArchetypeStructured.inl.hpp
        LIBRARIES ${PROJECT_NAME}::Engine.Serialization.Structures ${PROJECT_NAME}::Engine.Async ${PROJECT_NAME}::Engine.Scheduler ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Serialization.Structures ${PROJECT_NAME}::Engine.Assets
)
# shortcut: Actor/Actor.cpp imports Engine.Level (Level) and Engine.Scene (SceneBase, IComponentRegisterContext), both above ACS so linked as .Module, and textually includes Heliogrim (Session, Universe), resolved through Heliogrim's link of the executables; drop when actor spawning moves above ACS
target_link_libraries(${PROJECT_NAME}-Engine.ACS PRIVATE ${PROJECT_NAME}::Engine.Level.Module ${PROJECT_NAME}::Engine.Scene.Module)