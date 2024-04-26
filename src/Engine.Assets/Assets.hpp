#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Core/Module/CoreModule.hpp>

namespace hg::engine::assets {
	class IAssetRegistry;
	class AssetFactory;
}

namespace hg::engine {
	class Assets final :
		public core::CoreModule {
	public:
		using this_type = Assets;

		using IAssetRegistry = ::hg::engine::assets::IAssetRegistry;
		using AssetFactory = ::hg::engine::assets::AssetFactory;

	public:
		Assets(nmpt<Engine> engine_);

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
		uptr<AssetFactory> _factory;

	public:
		[[nodiscard]] nmpt<IAssetRegistry> getRegistry() const noexcept;

		[[nodiscard]] nmpt<AssetFactory> getFactory() const noexcept;
	};
}
