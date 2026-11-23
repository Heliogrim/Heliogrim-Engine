
# Setup
make_library_project(
        LIB_NAME Engine.Assets.Database
        LIB_FOLDER "${IDE_FOLDER}/Engine/Assets"
        LIBRARIES ${PROJECT_NAME}::Engine.Assets ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Assets.System ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts
)
# shortcut: AssetDatabase.cpp imports Engine.Core (engine access), drop when the service-locator design lands
target_link_libraries(${PROJECT_NAME}-Engine.Assets.Database PRIVATE ${PROJECT_NAME}::Engine.Core.Module)
