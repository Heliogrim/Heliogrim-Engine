#pragma once

#include <atomic>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

namespace hg::engine::gfx::material {
	template <typename StoredType_>
	struct MaterialParameterStorageTraits {
		using data_type = std::decay_t<StoredType_>;

		using ref_type = ref<data_type>;
		using cref_type = cref<data_type>;

		using store_type = std::conditional_t<std::is_pointer_v<StoredType_>, StoredType_, data_type>;
	};

	template <typename StoredType_>
	struct MaterialParameterStorageTraits<std::atomic<StoredType_>> {
		using data_type = std::decay_t<StoredType_>;

		using ref_type = data_type;
		using cref_type = data_type;

		using store_type = std::atomic<StoredType_>;
	};

	/**/

	class MaterialParameterStorageBase :
		public InheritBase<MaterialParameterStorageBase> {
	public:
		using this_type = MaterialParameterStorageBase;

	protected:
		constexpr MaterialParameterStorageBase() noexcept = default;

	public:
		constexpr ~MaterialParameterStorageBase() noexcept override = default;
	};

	template <typename StoredType_>
	class MaterialParameterStorage final :
		public InheritMeta<MaterialParameterStorage<StoredType_>, MaterialParameterStorageBase> {
	public:
		using this_type = MaterialParameterStorage<StoredType_>;

		template <typename Type_>
		using trait_type_t = MaterialParameterStorageTraits<Type_>;
		using trait_type = MaterialParameterStorageTraits<StoredType_>;

	public:
		MaterialParameterStorage() noexcept = default;

		~MaterialParameterStorage() noexcept override = default;

	private:
		typename trait_type::store_type _store;

	public:
		template <typename Tx_ = StoredType_>
		[[nodiscard]] typename trait_type_t<Tx_>::cref_type get() const noexcept {
			return _store;
		}

		template <typename Tx_ = typename trait_type_t<StoredType_>::data_type> requires
			(not std::is_pointer_v<typename trait_type_t<Tx_>::store_type>) &&
			std::is_move_assignable_v<typename trait_type_t<Tx_>::data_type>
		void set(typename trait_type_t<Tx_>::data_type&& value_) {
			_store = std::forward<typename trait_type_t<Tx_>::data_type>(value_);
		}

		template <typename Tx_ = typename trait_type_t<StoredType_>::data_type> requires
			std::is_pointer_v<typename trait_type_t<Tx_>::store_type> &&
			std::is_move_assignable_v<typename trait_type_t<Tx_>::store_type> &&
			(
				std::is_same_v<
					typename trait_type_t<StoredType_>::store_type, typename trait_type_t<Tx_>::store_type
				> ||
				std::is_nothrow_convertible_v<
					typename trait_type_t<Tx_>::store_type,
					typename trait_type_t<StoredType_>::store_type
				>
			)
		void set(typename trait_type_t<Tx_>::store_type&& value_) {
			_store = std::forward<typename trait_type_t<Tx_>::store_type>(value_);
		}

		template <typename Tx_ = typename trait_type_t<StoredType_>::data_type> requires
			(not std::is_pointer_v<typename trait_type_t<Tx_>::store_type>) &&
			std::is_move_assignable_v<typename trait_type_t<Tx_>::data_type> &&
			std::is_trivially_copy_assignable_v<typename trait_type_t<Tx_>::data_type>
		void set(cref<typename trait_type_t<Tx_>::data_type> value_) {
			_store = value_;
		}
	};
}
