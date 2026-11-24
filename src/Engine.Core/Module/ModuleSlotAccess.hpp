#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import std;
#endif

namespace hg::engine::core {
	class Modules;

	/**
	 * The single gateway to the private `_moduleSlot` of an engine module.
	 *
	 * @details A module declares `static constinit std::atomic<ptr<T>> _moduleSlot` (or `ptr<const T>` for read-only
	 *  access) and befriends this class. `Modules` publishes on add and retracts on remove; readers go through
	 *  `engine::module<T>()`.
	 */
	class ModuleSlotAccess {
	public:
		using this_type = ModuleSlotAccess;

	public:
		template <class Type_>
		using pointee = std::remove_pointer_t<typename decltype(Type_::_moduleSlot)::value_type>;

		template <class Type_>
		constexpr static bool published = requires {
			typename pointee<Type_>;
			requires std::is_same_v<
				std::remove_cv_t<decltype(Type_::_moduleSlot)>,
				std::atomic<ptr<pointee<Type_>>>
			>;
			requires std::derived_from<Type_, std::remove_const_t<pointee<Type_>>>;
		};

	public:
		template <class Type_> requires published<Type_>
		[[nodiscard]] static ptr<pointee<Type_>> load() noexcept {
			return Type_::_moduleSlot.load(std::memory_order_acquire);
		}

	private:
		friend class ::hg::engine::core::Modules;

		template <class Type_>
		static void publish(_In_ ptr<Type_> module_) noexcept {
			if constexpr (published<Type_>) {
				Type_::_moduleSlot.store(module_, std::memory_order_release);
			}
		}

		template <class Type_>
		static void retract() noexcept {
			if constexpr (published<Type_>) {
				Type_::_moduleSlot.store(nullptr, std::memory_order_release);
			}
		}
	};
}

namespace hg::engine {
	/**
	 * Read the registered engine module of `Type_`.
	 *
	 * @returns The module, or None while it is not registered.
	 */
	template <class Type_> requires core::ModuleSlotAccess::published<Type_>
	[[nodiscard]] Opt<ref<core::ModuleSlotAccess::pointee<Type_>>> module() noexcept {
		const auto slot = core::ModuleSlotAccess::load<Type_>();
		return (slot != nullptr) ? Some(*slot) : None;
	}
}
