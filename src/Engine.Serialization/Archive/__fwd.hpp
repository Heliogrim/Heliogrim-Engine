#pragma once

namespace hg::engine::resource {
	class Archive;
	class BufferArchive;

	class MemoryReadWriteArchive;
	using MemoryArchive = MemoryReadWriteArchive;

	class StorageReadWriteArchive;
	using StorageArchive = StorageReadWriteArchive;
}

namespace hg::engine::serialization {
	class LayoutArchiveBase;
	class StructuredArchive;
}
