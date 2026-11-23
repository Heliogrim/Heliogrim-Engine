#pragma once

#include <Engine.Assets.Database/AssetDatabase.hpp>
#include <Engine.Common/Sal.hpp>

#include "Core/Session.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine {
	class Engine;
}

namespace hg {
	class Heliogrim {
	public:
		static void instantiate();

		static void destroy();

	protected:
		static uptr<engine::Engine> _engine;

	public:
		[[nodiscard]] static nmpt<engine::Engine> getEngine() noexcept;

	public:
		[[nodiscard]] static Session getSession();

		[[nodiscard]] static Session getSession(std::nothrow_t) noexcept;

	public:
		/**
		 * Gets the Assets Module Interface
		 *
		 * @author Julius
		 * @date 27.09.2021
		 *
		 * @returns An AssetDatabase.
		 */
		[[nodiscard]] static AssetDatabase assets() noexcept;
	};
}
