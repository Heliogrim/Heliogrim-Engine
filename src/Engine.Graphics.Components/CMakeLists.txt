# Setup
make_library_project(
        LIB_NAME Engine.Graphics.Components
        LIB_FOLDER "${IDE_FOLDER}/Engine/Render"
        LIBRARIES ${PROJECT_NAME}::Engine.Serialization.Structures ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Assets.Database ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Serialization.Structures ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect
)
