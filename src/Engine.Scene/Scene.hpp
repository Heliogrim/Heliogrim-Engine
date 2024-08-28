#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Heliogrim/Component/SceneComponent.hpp>

#include "SceneBase.hpp"
#include "SceneSystem.hpp"

namespace hg::engine::scene {
	template <class Needle_, size_t Index_, class... HayStack_>
	struct index_of_type_impl;

	template <class Needle_, size_t Index_>
	struct index_of_type_impl<Needle_, Index_> {
		using type = void;
		constexpr static auto index = -1LL;
	};

	template <class Needle_, size_t Index_, class Check_, class... HayStack_>
	struct index_of_type_impl<Needle_, Index_, Check_, HayStack_...> {
		using next = index_of_type_impl<Needle_, Index_ + 1uLL, HayStack_...>;

		using type = std::conditional_t<(std::is_same_v<Needle_, Check_>), Check_, typename next::type>;
		constexpr static auto index = (std::is_same_v<Needle_, Check_>) ? (Index_) : (next::index);
	};

	/**/

	template <size_t Index_, class... TypeList_>
	struct type_of_index_impl;

	template <size_t Index_>
	struct type_of_index_impl<Index_> {
		using type = void;
	};

	template <size_t Index_, class Select_, class... TypeList_>
	struct type_of_index_impl<Index_, Select_, TypeList_...> {
		using next = type_of_index_impl<Index_ - 1uLL, TypeList_...>;
		using type = std::conditional_t<(Index_ == 0), Select_, typename next::type>;
	};

	/**/

	template <class Needle_, class... HayStack_>
	struct index_of_type {
		using type = typename index_of_type_impl<Needle_, 0uLL, HayStack_...>::type;
		constexpr static auto value = index_of_type_impl<Needle_, 0uLL, HayStack_...>::index;
	};

	/**/

	template <size_t Index_, class... TypeList_>
	struct type_of_index {
		using type = typename type_of_index_impl<Index_, TypeList_...>::type;
	};

	/**/

	template <typename Type_, typename... Types_>
	concept IsAnyOf = std::disjunction_v<std::is_same<Type_, Types_>...>;

	/**/

	template <typename... Types_>
	struct type_list;

	/**/

	template <typename Type_, typename Packed_>
	struct is_any_of_list;

	template <typename Type_, typename... Unpacked_>
	struct is_any_of_list<Type_, type_list<Unpacked_...>> {
		constexpr static auto value = IsAnyOf<Type_, Unpacked_...>;
	};

	/**/

	template <class Needle_, class PackedHayStack_>
	struct index_of_packed_type;

	template <class Needle_, class... UnpackedHayStack_>
	struct index_of_packed_type<Needle_, type_list<UnpackedHayStack_...>> {
		using type = typename index_of_type_impl<Needle_, 0uLL, UnpackedHayStack_...>::type;
		constexpr static auto value = index_of_type_impl<Needle_, 0uLL, UnpackedHayStack_...>::index;
	};

	/**/

	template <typename SceneType_, typename SystemType_>
	concept SceneHasSystem = std::derived_from<SceneType_, SceneBase> &&
		is_any_of_list<SystemType_, typename SceneType_::system_types>::value;
}

namespace hg::engine::scene {
	template <IsSceneSystem... Systems_>
	class Scene :
		public InheritMeta<Scene<Systems_...>, SceneBase> {
	public:
		using this_type = Scene<Systems_...>;

		using system_types = type_list<Systems_...>;
		constexpr static auto system_count = sizeof...(Systems_);

	protected:
		Scene() noexcept = default;

	public:
		Scene(this_type&&) = delete;

		Scene(const this_type&) = delete;

		~Scene() noexcept override {
			cleanup();
		}

	public:
		ref<Scene> operator=(mref<Scene>) = delete;

		ref<Scene> operator=(cref<Scene>) = delete;

	private:
		std::tuple<Systems_...> _typedSystems;
		Array<ptr<SceneSystemBase>, sizeof...(Systems_)> _denseSystems = {
			std::addressof(std::get<index_of_packed_type<Systems_, system_types>::value>(_typedSystems))...
		};

	private:
		template <typename Fn_>
		void unrollForEachFwd(Fn_&& fn_) {
			std::apply(std::forward<Fn_>(fn_), _typedSystems);
		}

		template <typename Fn_>
		void unrollForEachBwd(Fn_&& fn_) {
			::hg::todo_panic();
		}

	public:
		void prepare() override {
			unrollForEachFwd(
				[](auto& system_) {
					system_.prepare();
				}
			);
		}

		void cleanup() override {
			unrollForEachFwd/*unrollForEachBwd*/(
				[](auto& system_) {
					system_.cleanup();
				}
			);
		}

	private:
		template <typename Fn_>
		void forEachSystem(Fn_&& fn_) const {
			for (const auto* const system : _denseSystems) {
				fn_(system);
			}
		}

		template <typename Fn_>
		void forEachSystem(Fn_&& fn_) {
			for (auto* const system : _denseSystems) {
				fn_(system);
			}
		}

	public:
		template <IsSceneSystem SystemType_>
		[[nodiscard]] constexpr bool hasSceneSystem() const noexcept {
			return IsAnyOf<SystemType_, Systems_...>;
		}

		[[nodiscard]] bool hasSceneSystem(const ptr<const MetaClass> metaClass_) const noexcept override {
			for (const auto* const system : _denseSystems) {
				if (metaClass_->is(system->getMetaClass()->typeId())) {
					return true;
				}
			}
			return false;
		}

		template <IsSceneSystem SystemType_>
		[[nodiscard]] nmpt<SystemType_> getSceneSystem() const noexcept {
			static_assert(not std::is_void_v<typename index_of_type<SystemType_, Systems_...>::type>);
			return _denseSystems[index_of_type<SystemType_, Systems_...>::value];
		}

		template <size_t SystemIndex_>
		[[nodiscard]] nmpt<typename type_of_index<SystemIndex_, Systems_...>::type> getSceneSystem() const noexcept {
			static_assert(SystemIndex_ < system_count);
			return static_cast<ptr<typename type_of_index<SystemIndex_, Systems_...>::type>>(
				_denseSystems[SystemIndex_]
			);
		}

		[[nodiscard]] nmpt<SceneSystemBase> getSceneSystem(size_t index_) const noexcept override {
			if (index_ >= system_count) {
				return nullptr;
			}
			return _denseSystems[index_];
		}

		[[nodiscard]] nmpt<SceneSystemBase> getSceneSystem(
			const ptr<const MetaClass> metaClass_
		) const noexcept override {
			for (auto* const system : _denseSystems) {
				if (metaClass_->is(system->getMetaClass()->typeId())) {
					return nmpt<SceneSystemBase> { system };
				}
			}
			return nullptr;
		}

		void broadcast(SceneBroadcastFlags flags_) override {
			for (auto* const system : _denseSystems) {
				system->broadcast(flags_);
			}
		}

		void update(SceneUpdateFlags flags_) override {
			for (auto* const system : _denseSystems) {
				system->update(flags_);
			}
		}

		void postprocess(ScenePostProcessFlags flags_) override {
			for (auto* const system : _denseSystems) {
				system->postprocess(flags_);
			}
		}

	public:
		bool add(const ptr<SceneComponent> obj_) override {
			forEachSystem(
				[obj_](const ptr<SceneSystemBase> sys_) {
					sys_->add(obj_->getMetaClass(), std::span { &obj_, 1uLL });
				}
			);
			return true;
		}

		bool remove(const ptr<const SceneComponent> obj_) override {

			const auto* const metaClass = obj_->getMetaClass();
			forEachSystem(
				[metaClass, orphaned = obj_](const ptr<SceneSystemBase> sys_) {
					sys_->remove(metaClass, std::span { &orphaned, 1uLL });
				}
			);
			return true;
		}

	public:
		[[nodiscard]] virtual nmpt<IComponentRegisterContext> registerContext() noexcept override = 0;
	};
}
