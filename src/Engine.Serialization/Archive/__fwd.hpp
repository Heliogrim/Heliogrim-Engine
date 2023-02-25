#pragma once

namespace ember::engine::serialization {
    class Archive;

    class MemoryBaseArchive;
    class MemoryReadonlyArchive;
    class MemoryWriteonlyArchive;
    class MemoryReadWriteArchive;

    using MemoryArchive = MemoryReadWriteArchive;

    class BufferArchive;

    class SourceBaseArchive;
    class SourceReadonlyArchive;
    class SourceWriteonlyArchive;
    class SourceReadWriteArchive;

    using SourceArchive = SourceReadWriteArchive;

    class LayoutArchiveBase;
    class StructuredArchive;
}
