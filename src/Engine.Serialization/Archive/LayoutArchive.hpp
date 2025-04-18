#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"
#include "../Layout/DataLayout.hpp"

namespace hg::engine::serialization {
	class LayoutArchiveBase {
	public:
		using this_type = LayoutArchiveBase;

	protected:
		LayoutArchiveBase(_Inout_ ref<resource::Archive> archive_, cref<non_owning_rptr<DataLayoutBase>> layout_);

	public:
		LayoutArchiveBase(cref<this_type>) = delete;

		LayoutArchiveBase(mref<this_type>) noexcept = delete;

		~LayoutArchiveBase();

	private:
		nmpt<resource::Archive> _archive;

	public:
		[[nodiscard]] nmpt<resource::Archive> getArchive() const noexcept;

	private:
		const non_owning_rptr<DataLayoutBase> _layout;

	public:
		[[nodiscard]] cref<non_owning_rptr<DataLayoutBase>> getDataLayout() const noexcept;
	};

	template <typename SerializationType_> requires std::is_constructible_v<DataLayout<SerializationType_>>
	class TypedLayoutArchive final :
		public LayoutArchiveBase {
	public:
		using this_type = TypedLayoutArchive<SerializationType_>;

		using serialization_type = SerializationType_;
		using layout_type = typename DataLayout<serialization_type>::this_type;

	public:
		TypedLayoutArchive(_Inout_ ref<resource::Archive> archive_, cref<non_owning_rptr<layout_type>> layout_) :
			LayoutArchiveBase(archive_, layout_) {}

		~TypedLayoutArchive() {
			LayoutArchiveBase::~LayoutArchiveBase();
		}

	protected:
		void serialize(_In_ const ptr<const SerializationType_> value_) {

			std::span<u8, std::dynamic_extent> src {
				reinterpret_cast<ptr<u8>>(
					const_cast<ptr<SerializationType_>>(
						&const_cast<ref<SerializationType_>>(*value_)
					)
				),
				sizeof(SerializationType_)
			};

			const auto& defines { getDataLayout()->defines() };
			for (const auto& defined : defines) {
				defined._store(*getArchive(), defined, src);
			}

		}

		void deserialize(_Inout_ const ptr<SerializationType_> value_) {

			std::span<u8, std::dynamic_extent> src {
				reinterpret_cast<ptr<u8>>(value_),
				sizeof(SerializationType_)
			};

			const auto& defines { getDataLayout()->defines() };
			for (const auto& defined : defines) {
				defined._load(*getArchive(), defined, src);
			}

		}

	public:
		ref<this_type> operator<<(const ptr<const SerializationType_> object_) {
			serialize(object_);
			return *this;
		}

		ref<this_type> operator>>(const ptr<SerializationType_> object_) {
			deserialize(object_);
			return *this;
		}
	};

	template <typename LayoutType_> requires std::derived_from<LayoutType_, DataLayoutBase>
	class LayoutArchive final :
		public LayoutArchiveBase {
	public:
		using this_type = LayoutArchive<LayoutType_>;

		using layout_type = LayoutType_;

	public:
		LayoutArchive(const ptr<resource::Archive> archive_, cref<non_owning_rptr<layout_type>> layout_) :
			LayoutArchiveBase(archive_, layout_) {}

		~LayoutArchive() {
			LayoutArchiveBase::~LayoutArchiveBase();
		}

	protected:
		template <typename Type_>
		void serialize(_In_ const ptr<const Type_> value_) {

			std::span<u8, std::dynamic_extent> src {
				reinterpret_cast<ptr<u8>>(
					const_cast<ptr<Type_>>(
						&const_cast<ref<Type_>>(*value_)
					)
				),
				sizeof(Type_)
			};

			getDataLayout()->dispatch().store(*getArchive(), src);
		}

		template <typename Type_>
		void deserialize(_Inout_ const ptr<Type_> value_) {

			std::span<u8, std::dynamic_extent> dst {
				reinterpret_cast<ptr<u8>>(value_),
				sizeof(Type_)
			};

			getDataLayout()->dispatch().load(*getArchive(), dst);
		}

	public:
		template <typename Type_> requires std::is_constructible_v<DataLayout<Type_>>
		ref<this_type> operator<<(const ptr<const Type_> object_) {
			serialize<Type_>(object_);
			return *this;
		}

		template <typename Type_> requires std::is_constructible_v<DataLayout<Type_>>
		ref<this_type> operator>>(const ptr<Type_> object_) {
			deserialize<Type_>(object_);
			return *this;
		}
	};
}
