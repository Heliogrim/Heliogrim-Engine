#include "RenderSceneSystem.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/Pool/SceneResourcePool.hpp>

using namespace hg::engine::render;
using namespace hg;

RenderSceneSystem::RenderSceneSystem() = default;

RenderSceneSystem::~RenderSceneSystem() noexcept = default;

void RenderSceneSystem::prepare() {

	assert(_sceneResourcePool == nullptr);

	const auto graphics = Engine::getEngine()->getGraphics();
	_sceneResourcePool = make_uptr<gfx::SceneResourcePool>(graphics->getCurrentDevice());

	_sceneResourcePool->setup();
}

void RenderSceneSystem::cleanup() {

	if (_sceneResourcePool != nullptr) {
		_sceneResourcePool->destroy();
		_sceneResourcePool.reset();
	}

}

nmpt<engine::gfx::SceneResourcePool> RenderSceneSystem::getSceneResourcePool() const noexcept {
	return _sceneResourcePool.get();
}

void RenderSceneSystem::broadcast(scene::SceneBroadcastFlags flags_) {}

void RenderSceneSystem::update(scene::SceneUpdateFlags flags_) {

	for (auto& storage : _storage.storageMap.values()) {
		storage.second->forEachMut(
			[this](ref<RenderSceneSystemModel> model_) {
				model_.update(this);
			}
		);
	}

}

void RenderSceneSystem::postprocess(scene::ScenePostProcessFlags flags_) {
	for (auto& storage : _storage.storageMap.values()) {
		storage.second->remove(
			[](ref<const RenderSceneSystemModel> model_) {
				return model_.markedAsDeleted();
			}
		);
	}
}

ref<RenderSceneRegistry> RenderSceneSystem::getRegistry() noexcept {
	return _storage;
}

void RenderSceneSystem::add(const ptr<const MetaClass> meta_, std::span<const ptr<SceneComponent>> batch_) {

	for (const auto& mapping : _storage.mapping) {

		if (mapping.first != meta_) {
			continue;
		}

		auto* const storage = _storage.storageMap.at(mapping.second).get();
		for (auto* const src : batch_) {
			auto model = storage->add(src);
			model->create(this);
		}
	}

}

void RenderSceneSystem::remove(const ptr<const MetaClass> meta_, std::span<const ptr<const SceneComponent>> batch_) {

	// Warning: This is just a naive implementation with O^2 runtime complexity
	for (const auto& mapping : _storage.mapping) {
		if (mapping.first != meta_) {
			continue;
		}

		auto* const storage = _storage.storageMap.at(mapping.second).get();
		storage->forEachMut(
			[this, &batch_](ref<RenderSceneSystemModel> model_) {
				if (std::ranges::contains(batch_, model_.owner().get())) {
					model_.destroy(this);
					model_.markAsDeleted();
				}
			}
		);
	}
}

void RenderSceneSystem::clear() {
	for (auto& storage : _storage.storageMap.values()) {
		storage.second->forEachMut(
			[this](auto& model_) {
				model_.destroy(this);
				model_.markAsDeleted();
			}
		);
	}
}
