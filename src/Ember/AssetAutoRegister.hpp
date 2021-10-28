#pragma once

#include <type_traits>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>

#include "AssetDatabase.hpp"

namespace ember {

	namespace {
		template<class AssetType_>
		struct AssetAutoRegisterConstructor {
			/*
			template <typename... Args_>
			ptr<AssetType_> operator()(Args_ ... args_) const {
				return new AssetType_(_STD forward<Args_>(args_)...);
			}
			 */
			static ptr<Asset> make() {
				return new AssetType_();
			}
		};
	}

	template<class AssetType_>
	class AssetAutoRegister {
	public:
		using value_type = AssetAutoRegister<AssetType_>;
		using reference_type = ref<value_type>;
		using const_reference_type = cref<value_type>;

		static_assert(_STD is_base_of_v<Asset, AssetType_>,
					  "'AssetCrtpType_' should be derived from Asset class.");

		static_assert(_STD is_nothrow_convertible_v<ptr<AssetType_>, ptr<Asset>>,
					  "Pointer of type 'AssetCrtpType_' should be convertible to pointer of type 'Asset'.");

	public:
		inline static const bool auto_handle = AssetDatabase::autoRegister(
				&AssetAutoRegisterConstructor<AssetType_>::make);
	};

}

#ifndef AUTO_REGISTER_ASSET

#define AUTO_REGISTER_ASSET(AssetType_) \
    struct DOUBLE(AssetType_, Register) : public AssetAutoRegister<AssetType_> {};

#endif
