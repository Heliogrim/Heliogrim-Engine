# Setup
make_library_project(
        LIB_NAME Heliogrim.Actors
        LIB_FOLDER "${IDE_FOLDER}/Heliogrim"
        LIBRARIES ${PROJECT_NAME}::Engine.Serialization.Structures ${PROJECT_NAME}::Engine.Graphics.Components ${PROJECT_NAME}::Engine.Physics.Components ${PROJECT_NAME}::Engine.ACS ${PROJECT_NAME}::Engine.Assets.Database ${PROJECT_NAME}::Engine.Assets.Type ${PROJECT_NAME}::Engine.Serialization ${PROJECT_NAME}::Engine.Serialization.Structures ${PROJECT_NAME}::Engine.Resource.Archive ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Assets
)
