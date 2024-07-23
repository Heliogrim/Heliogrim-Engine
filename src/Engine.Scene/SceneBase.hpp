#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#include "SceneBroadcastFlags.hpp"
#include "ScenePostProcessFlags.hpp"
#include "SceneUpdateFlags.hpp"

#include "SceneSystem.hpp"

namespace hg {
	class SceneComponent;
	class IComponentRegisterContext;
}

namespace hg::engine::scene {
	class SceneSystemBase;
}

namespace hg::engine::scene {
	class macro_novtable SceneBase :
		public InheritBase<SceneBase> {
	public:
		using this_type = SceneBase;

	protected:
		SceneBase() noexcept;

	public:
		SceneBase(mref<SceneBase>) = delete;

		SceneBase(cref<SceneBase>) = delete;

		~SceneBase() noexcept override;

	public:
		ref<this_type> operator=(mref<this_type>) = delete;

		ref<this_type> operator=(cref<this_type>) = delete;

	public:
		virtual void prepare() = 0;

		virtual void cleanup() = 0;

	public:
		virtual void broadcast(SceneBroadcastFlags flags_) = 0;

		virtual void update(SceneUpdateFlags flags_) = 0;

		virtual void postprocess(ScenePostProcessFlags flags_) = 0;

	public:
		virtual bool add(const ptr<SceneComponent> obj_) = 0;

		virtual bool remove(const ptr<const SceneComponent> obj_) = 0;

	public:
		[[nodiscard]] virtual nmpt<IComponentRegisterContext> registerContext() noexcept = 0;

	public:
		[[nodiscard]] virtual bool hasSceneSystem(const ptr<const MetaClass> metaClass_) const noexcept = 0;

		[[nodiscard]] virtual nmpt<SceneSystemBase> getSceneSystem(size_t index_) const noexcept = 0;

		[[nodiscard]] virtual nmpt<SceneSystemBase> getSceneSystem(
			const ptr<const MetaClass> metaClass_
		) const noexcept = 0;

		template <IsSceneSystem SystemType_>
		[[nodiscard]] nmpt<SystemType_> getSceneSystem() const noexcept {
			return nmpt<SystemType_> {
				static_cast<ptr<SystemType_>>(getSceneSystem(SystemType_::getStaticMetaClass()).get())
			};
		}
	};
}
