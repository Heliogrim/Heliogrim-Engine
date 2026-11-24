
# Setup
make_library_project(
        LIB_NAME Engine.Serialization
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/IO"
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Diagnostics ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Logging ${PROJECT_NAME}::Engine.Resource.Archive
)
# shortcut: Load/RootKind.cpp reads SerializationModule::get() (Engine.ACS) for its migrations from below Engine.ACS, drop when the migrations are passed in
target_link_libraries(${PROJECT_NAME}-Engine.Serialization PRIVATE ${PROJECT_NAME}::Engine.ACS.Module)
