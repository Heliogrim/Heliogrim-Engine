#pragma once
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#include "SceneBroadcastFlags.hpp"
#include "ScenePostProcessFlags.hpp"
#include "SceneUpdateFlags.hpp"

namespace hg {
	class SceneComponent;
}

namespace hg::engine::acs {
	class RelocationTable;
}

namespace hg::engine::scene {
	class SceneSystemBase;

	template <typename Derived_>
	class SceneSystem;

	/**/

	template <typename Type_>
	concept IsSceneSystem = (sizeof(Type_) > 0uLL) && std::derived_from<Type_, SceneSystemBase>;
}

namespace hg::engine::scene {
	class macro_novtable SceneSystemBase :
		public InheritBase<SceneSystemBase> {
	public:
		using this_type = SceneSystemBase;

	public:
		~SceneSystemBase() noexcept override = default;

	public:
		virtual void prepare() = 0;

		virtual void cleanup() = 0;

	public:
		virtual void broadcast(SceneBroadcastFlags flags_) = 0;

		virtual void update(SceneUpdateFlags flags_) = 0;

		virtual void postprocess(ScenePostProcessFlags flags_) = 0;

	public:
		virtual void add(const ptr<const MetaClass> meta_, std::span<const ptr<SceneComponent>> batch_) = 0;

		virtual void remove(const ptr<const MetaClass> meta_, std::span<const ptr<const SceneComponent>> batch_) = 0;

		virtual void clear() = 0;

	public:
		/**
		 * Rebinds stored component pointers after the relocation barrier moved archetype slots.
		 */
		virtual void relocate(ref<const acs::RelocationTable> table_) {}
	};

	/**/

	template <typename Derived_>
	class SceneSystem :
		public InheritMeta<SceneSystem<Derived_>, SceneSystemBase> {
	public:
		using this_type = SceneSystem<Derived_>;
		using derived_type = Derived_;
	};
}
