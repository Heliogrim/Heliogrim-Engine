
# Setup
make_library_project(
        LIB_NAME Engine.Resource.Archive
        MODULE
        LIB_FOLDER "${IDE_FOLDER}/Engine/IO"
        # shortcut: the Storage*Archive.cpp files import Engine.Resource.Blob (Blob backs the storage archives) and Blob reaches Archive via Filesystem; drop when the Blob -> Filesystem -> Archive cycle is broken; Storage/PackageReadWriteArchive.cpp imports Engine.Resource.Package and Package links Archive, drop when the package archive moves into Engine.Resource.Package
        LIBRARIES ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Resource.Blob.Module ${PROJECT_NAME}::Engine.Resource.Package.Module
)