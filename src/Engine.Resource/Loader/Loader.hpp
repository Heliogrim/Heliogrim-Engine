#pragma once

#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#include "LoaderTraits.hpp"
#include "__fwd.hpp"
#include "../Manage/ResourceConcept.hpp"
#include "../Manage/UniqueResource.hpp"

namespace hg::engine::resource::loader {
	class LoaderBase;

	template <class Type_>
	concept IsLoader = std::derived_from<Type_, LoaderBase>;

	class __declspec(novtable) LoaderBase :
		public InheritBase<LoaderBase> {
	public:
		template <typename, typename, typename...>
		friend class ::hg::InheritMeta;

		template <typename>
		friend class ::hg::InheritBase;

		template <IsRequestValueType, IsResponseValueType>
		friend class Loader;

	public:
		using this_type = LoaderBase;

	private:
		LoaderBase() noexcept = default;

	public:
		~LoaderBase() noexcept override = default;

	public:
		[[nodiscard]] virtual smr<ResourceBase> operator()(
			ptr<assets::Asset> asset_,
			ptr<void> options_
		) = 0;
	};

	/**/

	namespace {
		template <typename RequestType_, typename ResponseType_, bool Streamable_>
		struct LoaderTypeBase {
		public:
			using this_type = LoaderTypeBase<RequestType_, ResponseType_, Streamable_>;

			using traits = LoaderTraits<RequestType_, ResponseType_>;

		public:
			virtual ~LoaderTypeBase() = default;

			[[nodiscard]] virtual typename traits::response::type operator()(
				mref<typename traits::request::type> request_,
				mref<typename traits::request::options> options_
			) = 0;
		};

		template <typename RequestType_, typename ResponseType_>
		struct LoaderTypeBase<RequestType_, ResponseType_, true> {
		public:
			using this_type = LoaderTypeBase<RequestType_, ResponseType_, true>;

			using traits = LoaderTraits<RequestType_, ResponseType_>;

		public:
			virtual ~LoaderTypeBase() = default;

			[[nodiscard]] virtual typename traits::response::type operator()(
				mref<typename traits::request::type> request_,
				mref<typename traits::request::options> options_
			) = 0;

			[[nodiscard]] virtual typename traits::stream_response::type operator()(
				mref<typename traits::stream_request::type> request_,
				mref<typename traits::stream_request::options> options_
			) = 0;
		};
	}

	/**/

	template <IsRequestValueType RequestType_, IsResponseValueType ResponseType_>
	class __declspec(novtable) Loader :
		public InheritMeta<
			Loader<RequestType_, ResponseType_>,
			LoaderBase,
			LoaderTypeBase<RequestType_, ResponseType_, assets::IsStreamableAsset<RequestType_>>
		> {
	public:
		template <typename, typename, typename...>
		friend class ::hg::InheritMeta;

		template <typename>
		friend class ::hg::InheritBase;

		template <
			IsRequestValueType,
			IsResponseValueType,
			typename,
			class,
			class,
			class,
			class
		>
		friend class LoaderChainBase;

	public:
		using this_type = Loader<RequestType_, ResponseType_>;
		using type_base = LoaderTypeBase<RequestType_, ResponseType_, assets::IsStreamableAsset<RequestType_>>;

		using traits = LoaderTraits<RequestType_, ResponseType_>;

	private:
		Loader() noexcept = default;

	public:
		~Loader() noexcept override = default;

	public:
		[[nodiscard]] smr<ResourceBase> operator()(
			ptr<assets::Asset> asset_,
			ptr<void> options_
		) override {

			using request_type = typename traits::request::type;
			using options_type = typename traits::request::options;

			static_assert(
				std::derived_from<std::remove_pointer_t<request_type>, std::remove_pointer_t<decltype(asset_)>> ||
				std::is_convertible_v<decltype(asset_), request_type>
			);
			static_assert(
				std::is_void_v<std::remove_pointer_t<decltype(options_)>> ||
				std::is_convertible_v<decltype(options_), ptr<options_type>>
			);

			if constexpr (std::derived_from<
				typename traits::response::type,
				smr<typename traits::response::value_type>
			>) {
				return static_cast<const ptr<type_base>>(this)->operator()(
					std::move(static_cast<request_type>(asset_)),
					std::move(*static_cast<ptr<options_type>>(options_))
				);
			}

			/*
			return make_smr(
			    static_cast<const ptr<type_base>>(this)->operator()(
			        std::move(static_cast<request_type>(asset_)),
			        std::move(*static_cast<ptr<options_type>>(options_))
			    )
			);
			 */
			return {};
		}
	};
}
