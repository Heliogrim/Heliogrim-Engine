#pragma once
#include <Engine.Common/Collection/DenseMap.hpp>

#include "../Provider.hpp"

namespace hg::engine::cfg {
	class ProjectProvider final :
		public Provider {
	public:
		using this_type = ProjectProvider;

	public:
		~ProjectProvider() override = default;

	private:
		DenseMap<InternalConfigKey, ConfigEntry> _props;

	public:
		void add(mref<ConfigEntry> entry_) override;

		[[nodiscard]] bool contains(InternalConfigKey key_) const noexcept override;

		[[nodiscard]] ConfigValue get(InternalConfigKey key_) const override;

		[[nodiscard]] bool set(InternalConfigKey key_, mref<ConfigValue> next_) noexcept override;

		bool remove(InternalConfigKey key_) override;

	public:
		[[nodiscard]] bool isConfigType(InternalConfigKey key_, type_id typeId_) const noexcept override;
	};
}
