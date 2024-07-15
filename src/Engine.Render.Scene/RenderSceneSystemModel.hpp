#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

namespace hg {
	class SceneComponent;
}

namespace hg::engine::render {
	class RenderSceneSystem;
}

namespace hg::engine::render {
	class __declspec(novtable) RenderSceneSystemModel :
		public InheritBase<RenderSceneSystemModel> {
	public:
		using this_type = RenderSceneSystemModel;

	protected:
		RenderSceneSystemModel() noexcept = default;

		explicit RenderSceneSystemModel(nmpt<SceneComponent> owner_) noexcept;

	public:
		RenderSceneSystemModel(this_type&&) noexcept = default;

		RenderSceneSystemModel(const this_type&) = delete;

		~RenderSceneSystemModel() noexcept override = default;

	public:
		ref<this_type> operator=(mref<this_type>) noexcept = default;

		ref<this_type> operator=(cref<this_type>) = delete;

	protected:
		nmpt<SceneComponent> _owner;

	public:
		[[nodiscard]] const nmpt<SceneComponent> owner() const noexcept;

	public:
		virtual void create(const ptr<RenderSceneSystem> system_/* const ptr<SceneComponent> origin_ */) = 0;

		virtual void update(const ptr<RenderSceneSystem> system_/* const ptr<SceneComponent> origin_ */) = 0;

		virtual void destroy(const ptr<RenderSceneSystem> system_) = 0;

	private:
		bool _markAsDeleted;

	public:
		[[nodiscard]] bool markedAsDeleted() const noexcept;

		[[nodiscard]] bool markAsDeleted() noexcept;
	};
}
