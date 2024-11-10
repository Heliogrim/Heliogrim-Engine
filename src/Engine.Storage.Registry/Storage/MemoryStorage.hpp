#pragma once

#include <functional>
#include <span>
#include <utility>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Types.hpp>

#include "../IStorage.hpp"

namespace hg::engine::storage {
	class StorageIo;
}

namespace hg::engine::storage::system {
	class MemoryStorage final :
		public InheritMeta<MemoryStorage, IStorage> {
	public:
		friend class ::hg::engine::storage::StorageIo;

	public:
		using this_type = MemoryStorage;
		class MemoryObject;

	public:
		MemoryStorage() noexcept;

		MemoryStorage(
			mref<MemoryObject> memoryObject_,
			bool readable_,
			bool writeable_,
			bool randomReadable_,
			bool randomWritable_
		) noexcept;

		~MemoryStorage() override = default;

	public:
		[[nodiscard]] constexpr bool isPrimaryStorage() const noexcept override {
			return true;
		}

	private:
		u8 _readable : 1;
		u8 _writeable : 1;
		u8 _randomReadable : 1;
		u8 _randomWritable : 1;

	public:
		[[nodiscard]] bool isReadable() const noexcept override;

		[[nodiscard]] bool isWriteable() const noexcept override;

		[[nodiscard]] bool isRandomAccess() const noexcept override;

		[[nodiscard]] bool isRandomReadable() const noexcept override;

		[[nodiscard]] bool isRandomWritable() const noexcept override;

	public:
		class MemoryObject {
		public:
			using dealloc_callback = std::function<void(ptr<_::byte>, size_t)>;

		public:
			MemoryObject() noexcept :
				_base(nullptr),
				_size(0uLL),
				_touched(0uLL),
				_dealloc() {}

			MemoryObject(
				mref<ptr<_::byte>> memoryBase_,
				mref<size_t> memorySize_,
				mref<size_t> touchedSize_,
				mref<dealloc_callback> dealloc_
			) noexcept :
				_base(std::move(memoryBase_)),
				_size(std::move(memorySize_)),
				_touched(std::move(touchedSize_)),
				_dealloc(std::move(dealloc_)) {}

			MemoryObject(const MemoryObject&) = delete;

			MemoryObject(mref<MemoryObject> other_) noexcept :
				_base(std::exchange(other_._base, nullptr)),
				_size(std::exchange(other_._size, 0uLL)),
				_touched(std::exchange(other_._touched, 0uLL)),
				_dealloc(std::move(other_._dealloc)) {}

			~MemoryObject() {
				if (not empty()) {
					destroy();
				}
			}

		private:
			ptr<_::byte> _base;
			size_t _size;
			size_t _touched;

			dealloc_callback _dealloc;

		public:
			[[nodiscard]] constexpr bool empty() const noexcept {
				return _base == nullptr;
			}

			void destroy() {
				_dealloc(std::exchange(_base, nullptr), std::exchange(_size, 0uLL));
			}

			[[nodiscard]] constexpr std::span<_::byte> span() const noexcept {
				return std::span<_::byte> { _base, _size };
			}

		public:
			constexpr size_t markAsTouched(const size_t touchedSize_) noexcept {
				_touched = std::min({ _touched, touchedSize_, _size });
				return _touched;
			}

			constexpr void setAsTouched(const size_t forceTouchedSize_) {
				::hg::assertrt(forceTouchedSize_ <= _size);
				_touched = forceTouchedSize_;
			}

			constexpr void resetTouched() noexcept {
				_touched = 0uLL;
			}

			[[nodiscard]] constexpr size_t touched() const noexcept {
				return _touched;
			}

			[[nodiscard]] constexpr std::span<_::byte> touchedSpan() const noexcept {
				return std::span<_::byte> { _base, _touched };
			}
		};

	private:
	public:
		MemoryObject _memory;
	};
}
