#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Core;
#endif

namespace hg::engine::assets {
	class IAssetRegistry;
}

namespace hg::engine {
	class Assets final :
		public core::CoreModule {
	public:
		using this_type = Assets;

		using IAssetRegistry = ::hg::engine::assets::IAssetRegistry;

	public:
		explicit Assets(ref<Engine> engine_);

		Assets(cref<this_type>) = delete;

		Assets(mref<this_type>) noexcept = delete;

		~Assets() override;

	public:
		void setup() override;

		void start() override;

		void stop() override;

		void destroy() override;

	private:
		uptr<IAssetRegistry> _registry;

	public:
		[[nodiscard]] nmpt<IAssetRegistry> getRegistry() const noexcept;
	};
}
