
# Setup
make_library_project(
        LIB_NAME Engine.Assets
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/Assets"
        LIBRARIES ${PROJECT_NAME}::Engine.Filesystem ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Storage.Registry
)
# shortcut: Assets.cpp imports Engine.Assets.System (owns the asset registry) and Engine.Assets.System links Engine.Assets, drop when the registry ownership moves out of Engine.Assets
target_link_libraries(${PROJECT_NAME}-Engine.Assets PRIVATE ${PROJECT_NAME}::Engine.Assets.System.Module)