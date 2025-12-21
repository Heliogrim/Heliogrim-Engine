#pragma once

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflect/Meta/MetaClass.hpp>
#include <Engine.Scene/SceneBase.hpp>
#include <Heliogrim/IComponentRegisterContext.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>

namespace hg::engine::scene {
	template <typename Scene_>
	concept HasBatchAdd = requires(
		ref<Scene_> obj_,
		const __restricted_ptr<const MetaClass> metaClass_,
		std::span<const ptr<SceneComponent>> components_
	) {
			{ obj_.add(metaClass_, components_) };
		};

	template <typename Scene_>
	concept HasBatchRemove = requires(
		ref<Scene_> obj_,
		const __restricted_ptr<const MetaClass> metaClass_,
		std::span<const ptr<const SceneComponent>> components_
	) {
			{ obj_.remove(metaClass_, components_) };
		};

	template <typename Scene_>
	class SceneComponentQueue :
		public IComponentRegisterContext {
	public:
		using this_type = SceneComponentQueue<Scene_>;
		using scene_type = Scene_;
		using MetaClassKey = __restricted_ptr<const MetaClass>;

	public:
		virtual ~SceneComponentQueue() {
			commitChanges();
		}

	private:
		DenseMap<MetaClassKey, Vector<ptr<SceneComponent>>> _createQueue;
		DenseMap<MetaClassKey, Vector<ptr<SceneComponent>>> _removeQueue;

	protected:
		template <std::derived_from<SceneBase> Derived_ = Scene_> requires std::is_base_of_v<SceneComponentQueue<Derived_>, Derived_>
		void commitChanges() {

			if constexpr (HasBatchRemove<Derived_>) {
				for (auto&& batch : _removeQueue) {
					static_cast<ref<Derived_>>(*this).remove(batch.first, batch.second);
				}
			} else {
				for (auto&& batch : _removeQueue) {
					for (const auto* const component : batch.second) {
						static_cast<ref<SceneBase>>(static_cast<ref<Derived_>>(*this)).remove(component);
					}
				}
			}

			_removeQueue.clear();

			/**/

			if constexpr (HasBatchAdd<Derived_>) {
				for (auto&& batch : _createQueue) {
					static_cast<ref<Derived_>>(*this).add(batch.first, batch.second);
				}
			} else {
				for (auto&& batch : _createQueue) {
					for (auto* const component : batch.second) {
						static_cast<ref<SceneBase>>(static_cast<ref<Derived_>>(*this)).add(component);
					}
				}
			}

			_createQueue.clear();
		}

	public:
		void add(ptr<HierarchyComponent> component_) override {

			auto* const component = Cast<SceneComponent>(component_);
			if (component == nullptr) {
				return;
			}

			const auto meta = component->getMetaClass();
			const auto foundIt = _createQueue.find(meta);

			if (foundIt != _createQueue.end()) {
				foundIt->second.emplace_back(component);
				return;
			}

			_createQueue.emplace(meta, Vector<ptr<SceneComponent>> { component });
		}

		void add(ref<const ComponentHierarchy> hierarchy_) override {
			::hg::todo_panic();
		}

		void remove(ptr<HierarchyComponent> component_) override {

			auto* const component = Cast<SceneComponent>(component_);
			if (component == nullptr) {
				return;
			}

			const auto meta = component->getMetaClass();
			const auto foundIt = _removeQueue.find(meta);

			if (foundIt != _removeQueue.end()) {
				foundIt->second.emplace_back(component);
				return;
			}

			_removeQueue.emplace(meta, Vector<ptr<SceneComponent>> { component });
		}

		void remove(ref<const ComponentHierarchy> hierarchy_) override {
			::hg::todo_panic();
		}
	};
}
