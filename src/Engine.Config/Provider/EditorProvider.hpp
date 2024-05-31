#pragma once
#include <Engine.Common/Collection/DenseMap.hpp>

#include "../Provider.hpp"

namespace hg::editor::cfg {
	class EditorProvider final :
		public engine::cfg::Provider {
	public:
		using this_type = EditorProvider;

	public:
		~EditorProvider() override = default;

	private:
		DenseMap<engine::cfg::InternalConfigKey, engine::cfg::ConfigEntry> _props;

	public:
		void add(mref<engine::cfg::ConfigEntry> entry_) override;

		[[nodiscard]] bool contains(engine::cfg::InternalConfigKey key_) const noexcept override;

		[[nodiscard]] engine::cfg::ConfigValue get(engine::cfg::InternalConfigKey key_) const override;

		[[nodiscard]] bool set(
			engine::cfg::InternalConfigKey key_,
			mref<engine::cfg::ConfigValue> next_
		) noexcept override;

		bool remove(engine::cfg::InternalConfigKey key_) override;

	public:
		[[nodiscard]] bool isConfigType(engine::cfg::InternalConfigKey key_, type_id typeId_) const noexcept override;
	};
}
