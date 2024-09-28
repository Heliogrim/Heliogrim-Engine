#pragma once

namespace hg::engine::resource {
	class Archive;
	class BufferArchive;

	class MemoryBaseArchive;
	class MemoryReadonlyArchive;
	class MemoryWriteonlyArchive;
	class MemoryReadWriteArchive;

	using MemoryArchive = MemoryReadWriteArchive;

	class StorageBaseArchive;
	class StorageReadonlyArchive;
	class StorageWriteonlyArchive;
	class StorageReadWriteArchive;

	using StorageArchive = StorageReadWriteArchive;
}
