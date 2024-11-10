#pragma once

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "RefCounted.hpp"
#include "../Move.hpp"
#include "../Wrapper.hpp"
#include "../Memory/MemoryPointer.hpp"

namespace hg {
	template <typename Ty_>
	class RefCounted;

	/**/

	class RefCountedBlockBase {
	public:
		virtual ~RefCountedBlockBase() noexcept = default;

		virtual void destroy_obj() = 0;
	};

	template <typename Ty_>
	class RefCountedBlock final :
		public RefCountedBlockBase {
	public:
		template <typename>
		friend class RefCountedBlock;

		friend class ::hg::RefCounted<Ty_>;

	public:
		using this_type = RefCountedBlock<Ty_>;

		using counter_type = u16;
		using storage_type = mpt<Ty_>;

	public:
		template <typename... Args_>
		explicit RefCountedBlock(Args_&&... args_) :
			RefCountedBlockBase(),
			_refs(1),
			_obj(storage_type::make(std::forward<Args_>(args_)...)) {}

		RefCountedBlock(this_type&&) = delete;

		RefCountedBlock(const this_type&) = delete;

		~RefCountedBlock() override {
			assert(_refs == 0);
		}

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

		ref<this_type> operator=(mref<this_type>) = delete;

	private:
		counter_type _refs;
		storage_type _obj;

	private:
		constexpr void throw_if_zero() const {
			if (_refs <= 0) {
				throw std::runtime_error("Failed to access ref counted object with base ref of zero.");
			}
		}

		constexpr void proxy_destroy_obj() {
			static_cast<ptr<RefCountedBlockBase>>(this)->destroy_obj();
		}

		void destroy_obj() override {
			if constexpr (not std::is_void_v<Ty_>) {
				_obj.destroy();
			}
		}

	public:
		constexpr void inc_nz() noexcept {
			if (_refs != 0) {
				++_refs;
			}
		}

		[[nodiscard]] bool dec_nz() {
			if (_refs == 0) {
				return false;
			}

			if (--_refs == 0) {
				proxy_destroy_obj();
				return true;
			}

			return false;
		}

		[[nodiscard]] counter_type ref_count() const noexcept {
			return _refs;
		}

	public:
		[[nodiscard]] constexpr typename storage_type::non_owning_type get() noexcept {
			return _obj;
		}

		[[nodiscard]] constexpr storage_type unsafe_release() noexcept {
			return std::move(_obj);
		}
	};

	/**/

	template <typename Ty_>
	class RefCounted final {
	public:
		template <typename>
		friend class RefCounted;

	public:
		using this_type = RefCounted<Ty_>;

		using block_type = RefCountedBlock<Ty_>;
		using storage_type = nmpt<Ty_>;

	public:
		template <typename... Args_> requires std::is_constructible_v<Ty_, Args_&&...>
		[[nodiscard]] constexpr static this_type create(Args_&&... args_) noexcept {

			auto alloc = std::allocator<block_type> {};
			auto alloc_traits = std::allocator_traits<std::allocator<block_type>> {};

			auto block = alloc.allocate(1);
			alloc_traits.construct(alloc, block, std::forward<Args_>(args_)...);

			return this_type { std::move(block) };
		}

	private:
		template <typename BlockType_> requires std::is_constructible_v<nmpt<block_type>, BlockType_>
		explicit constexpr RefCounted(BlockType_&& block_) noexcept :
			_base(std::forward<BlockType_>(block_)),
			_obj(block_get_or_null()) {}

	public:
		constexpr RefCounted() noexcept :
			_base(nullptr),
			_obj(nullptr) {}

		constexpr RefCounted(::std::nullptr_t) noexcept :
			RefCounted() {}

		template <typename Ux_>
		constexpr RefCounted(RefCounted<Ux_>&& other_) noexcept requires std::is_convertible_v<ref<Ux_>, ref<Ty_>> :
			_base(_void_cast<block_type>(std::exchange(other_._base, nullptr).get())),
			_obj(::hg::move(other_._obj)) {}

		constexpr RefCounted(const this_type& other_) noexcept :
			_base(other_._base),
			_obj(other_._obj) {
			inc_not_null();
		}

		constexpr RefCounted(this_type&& other_) noexcept :
			_base(std::exchange(other_._base, nmpt<block_type> { nullptr })),
			_obj(std::exchange(other_._obj, storage_type { nullptr })) {}

		~RefCounted() {
			dec_not_null();
		}

	public:
		ref<this_type> operator=(this_type&& other_) noexcept {
			if (std::addressof(other_) != this) {
				dec_not_null();
				_base = std::exchange(other_._base, nmpt<block_type> { nullptr });
				_obj = std::exchange(other_._obj, storage_type { nullptr });
			}
			return *this;
		}

		ref<this_type> operator=(const this_type& other_) noexcept {
			if (std::addressof(other_) != this) {
				dec_not_null();
				_base = other_._base;
				_obj = other_._obj;
				inc_not_null();
			}
			return *this;
		}

	private:
		nmpt<block_type> _base;
		storage_type _obj;

	private:
		constexpr void destroy_block() {

			auto alloc = std::allocator<block_type> {};
			auto alloc_traits = std::allocator_traits<std::allocator<block_type>> {};

			alloc_traits.destroy(alloc, _base.get());
			alloc.deallocate(_base.get(), 1uLL);

			_base = nullptr;
		}

		constexpr void inc_not_null() noexcept {
			if (_base != nullptr) {
				_base->inc_nz();
			}
		}

		void dec_not_null() {
			if (_base == nullptr) {
				return;
			}

			if (_base->dec_nz()) {
				destroy_block();
			}
		}

		[[nodiscard]] constexpr storage_type block_get_or_null() noexcept {
			return _base != nullptr ? _base->get() : storage_type { nullptr };
		}

		constexpr void throw_if_null() const {
			if (_base == nullptr) {
				throw std::runtime_error("Failed to access ref counted object with ref count of zero.");
			}
		}

	public:
		[[nodiscard]] typename block_type::counter_type ref_count() const noexcept {
			return _base == nullptr ? 0 : _base->ref_count();
		}

		void reset() {
			_obj = nullptr;
			dec_not_null();
			_base = nullptr;
		}

		[[nodiscard]] typename block_type::storage_type release() noexcept {
			if (_base != nullptr && _base->ref_count() == 1) {

				auto tmp = _base->unsafe_release();
				dec_not_null();
				return tmp;
			}
			return typename block_type::storage_type { nullptr };
		}

	public:
		[[nodiscard]] constexpr ptr<Ty_> operator->() const {
			throw_if_null();
			return _obj.get();
		}

		template <typename Tx_ = Ty_> requires (not std::is_void_v<Ty_>)
		[[nodiscard]] constexpr mref<Tx_> operator*() && {
			throw_if_null();
			return *_obj;
		}

		template <typename Tx_ = Ty_> requires (not std::is_void_v<Ty_>)
		[[nodiscard]] constexpr cref<Tx_> operator*() const & {
			throw_if_null();
			return *_obj;
		}

		template <typename Tx_ = Ty_> requires (not std::is_void_v<Ty_>)
		[[nodiscard]] constexpr ref<Tx_> operator*() & {
			throw_if_null();
			return *_obj;
		}

	public:
		[[nodiscard]] constexpr bool operator==(std::nullptr_t) const noexcept {
			return _obj == nullptr;
		}
	};
}
