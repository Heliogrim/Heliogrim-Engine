
# Setup
make_library_project(
        LIB_NAME Engine.Resource.Archive
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/IO"
        # shortcut: the Storage*Archive.cpp files import Engine.Resource.Blob (Blob backs the storage archives) and Blob reaches Archive via Filesystem; drop when the Blob -> Filesystem -> Archive cycle is broken
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Resource.Blob.Module
)