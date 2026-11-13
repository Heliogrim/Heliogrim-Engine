#pragma once

#include <utility>
#include <Engine.Asserts/Todo.hpp>
#include <Engine.Async/Primitive/SpinLock.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflect/Meta/MetaClass.hpp>
#include <Engine.Scene/SceneBase.hpp>
#include <Heliogrim/IComponentRegisterContext.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>
#include <Heliogrim/Component/SceneComponent.hpp>

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
		// Producers (actor spawn and destroy, level load) run on any worker while the scene commits.
		concurrent::SpinMutex _lck;
		DenseMap<MetaClassKey, Vector<ptr<SceneComponent>>> _createQueue;
		DenseMap<MetaClassKey, Vector<ptr<SceneComponent>>> _removeQueue;

	protected:
		template <std::derived_from<SceneBase> Derived_ = Scene_> requires std::is_base_of_v<SceneComponentQueue<Derived_>, Derived_>
		void commitChanges() {

			// Taken out under the lock and applied outside it: the scene systems may queue again while applying.
			decltype(_createQueue) createQueue {};
			decltype(_removeQueue) removeQueue {};
			{
				const concurrent::SpinLock lock { _lck };
				std::swap(createQueue, _createQueue);
				std::swap(removeQueue, _removeQueue);
			}

			if constexpr (HasBatchRemove<Derived_>) {
				for (auto&& batch : removeQueue) {
					static_cast<ref<Derived_>>(*this).remove(batch.first, batch.second);
				}
			} else {
				for (auto&& batch : removeQueue) {
					for (const auto* const component : batch.second) {
						static_cast<ref<SceneBase>>(static_cast<ref<Derived_>>(*this)).remove(component);
					}
				}
			}

			/**/

			if constexpr (HasBatchAdd<Derived_>) {
				for (auto&& batch : createQueue) {
					static_cast<ref<Derived_>>(*this).add(batch.first, batch.second);
				}
			} else {
				for (auto&& batch : createQueue) {
					for (auto* const component : batch.second) {
						static_cast<ref<SceneBase>>(static_cast<ref<Derived_>>(*this)).add(component);
					}
				}
			}
		}

	public:
		void add(ptr<HierarchyComponent> component_) override {

			auto* const component = Cast<SceneComponent>(component_);
			if (component == nullptr) {
				return;
			}

			const auto meta = component->getMetaClass();
			const concurrent::SpinLock lock { _lck };
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
			const concurrent::SpinLock lock { _lck };
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
