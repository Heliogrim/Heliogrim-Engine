#pragma once

#include <Engine.Common/Sal.hpp>

#include "../Component/Component.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflect;
#endif

namespace hg::engine::render::graph {
	class Auxiliary {
	public:
		using this_type = Auxiliary;

	public:
		Auxiliary() noexcept = default;

		~Auxiliary();

	private:
		void tidy();

	private:
		CompactSet<ptr<Component>> _comps;

	public:
		[[nodiscard]] cref<CompactSet<ptr<Component>>> components() const noexcept;

		/**
		 * Add a component to the auxiliary set
		 *
		 * @param component_ A unique pointer tot the component to store
		 *
		 * @details This will unwrap the unique pointer internally and rely on
		 *  deletion after remove or on auxiliary destruction.
		 */
		void add(mref<uptr<Component>> component_) noexcept;

		/**
		 * Remove component from auxiliary set
		 *
		 * @param component_ A pointer to the component to remove
		 *
		 * @warning This will implicitly erase/delete the given component.
		 */
		void remove(_In_ mref<ptr<Component>> component_) noexcept;

	public:
		template <typename ComponentType_>
		[[nodiscard]] nmpt<ComponentType_> getFirstOfType() const noexcept {
			const auto compIt = std::ranges::find_if(
				_comps,
				[](const auto comp_) {
					return IsType<ComponentType_>(*comp_);
				}
			);
			return compIt != _comps.end() ?
				static_cast<const ptr<ComponentType_>>(*compIt) :
				nmpt<ComponentType_> {};
		}

		template <typename ComponentType_, typename Fn_>
		void eachOfType(Fn_&& fn_) const noexcept {
			for (auto* const comp : _comps) {
				if (IsType<ComponentType_>(*comp)) {
					fn_(static_cast<const ptr<ComponentType_>>(comp));
				}
			}
		}
	};
}
