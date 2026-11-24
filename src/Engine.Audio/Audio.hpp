#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Core;
#endif

namespace hg::engine {
	class Audio final :
		public core::CoreModule {
	private:
		friend class ::hg::engine::core::ModuleSlotAccess;

	public:
		using value_type = Audio;
		using reference_type = ref<value_type>;
		using const_reference_type = cref<value_type>;

	public:
		Audio(ref<Engine> engine_) noexcept;

		~Audio() noexcept override;

	private:
		static constinit std::atomic<ptr<Audio>> _moduleSlot;

	public:
		[[nodiscard]] static Opt<ref<Audio>> get() noexcept {
			return engine::module<Audio>();
		}

	public:
		void setup() override;

		void start() override;

		void stop() override;

		void destroy() override;

	private:
		/**
		 * Sets up the importer
		 *
		 * @author Julius
		 * @date 12.09.2021
		 */
		void registerImporter();

		void unregisterImporter();
	};
}
