#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Functional/FnRef.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Meta/MetaClass.hpp>
#include <Heliogrim/Component/SceneComponent.hpp>

#include "Engine.Common/Move.hpp"
#include "Engine.Common/Collection/Vector.hpp"

namespace hg::engine::scene {
	template <typename BaseModelType_>
	class macro_novtable SceneSystemTypeMappedStorageBase {
	public:
		virtual ~SceneSystemTypeMappedStorageBase() = default;

	public:
		[[nodiscard]] virtual ref<const MetaClass> getDataMetaClass() const noexcept = 0;

	public:
		virtual nmpt<BaseModelType_> add(ref<SceneComponent> src_) = 0;

		virtual void remove(ptr<const BaseModelType_>&& obj_) = 0;

		virtual void remove(FnRef<bool(ref<const BaseModelType_>)> selector_) = 0;

	public:
		virtual void forEach(FnRef<void(ref<const BaseModelType_>)> fn_) const = 0;

		virtual void forEachMut(FnRef<void(ref<BaseModelType_>)> fn_) = 0;
	};

	template <typename ModelType_, typename BaseModelType_> requires std::derived_from<ModelType_, BaseModelType_>
	class SceneSystemTypeMappedStorage :
		public SceneSystemTypeMappedStorageBase<BaseModelType_> {
	public:
		constexpr SceneSystemTypeMappedStorage() noexcept :
			_dataMetaClass(*ModelType_::getStaticMetaClass()),
			_denseData() {}

		~SceneSystemTypeMappedStorage() override = default;

	protected:
		ref<const MetaClass> _dataMetaClass;
		Vector<ModelType_> _denseData;

	public:
		[[nodiscard]] ref<const MetaClass> getDataMetaClass() const noexcept override {
			return _dataMetaClass;
		}

	public:
		nmpt<BaseModelType_> add(ref<SceneComponent> src_) override {
			_denseData.emplace_back(std::forward<decltype(src_)>(src_));
			return &_denseData.back();
		}

		void remove(ptr<const BaseModelType_>&& obj_) override {
			const auto* const self = static_cast<const ModelType_*>(obj_);
			::hg::assertd(self >= _denseData.data() && self < _denseData.data() + _denseData.size());
			const auto removeIt = _denseData.begin() + (self - _denseData.data());

			_denseData.erase(removeIt);
		}

		void remove(FnRef<bool(ref<const BaseModelType_>)> selector_) override {
			auto orphaned = std::ranges::remove_if(_denseData, ::hg::move(selector_));
			_denseData.erase(orphaned.begin(), orphaned.end());
		}

		void forEach(FnRef<void(ref<const BaseModelType_>)> fn_) const override {
			for (const auto& stored : _denseData) {
				fn_(stored);
			}
		}

		template <typename Fn_> requires
			(not std::is_invocable_v<Fn_, ref<const BaseModelType_>>) &&
			std::is_invocable_v<Fn_, ref<const ModelType_>>
		void forEach(Fn_&& fn_) const {
			for (const auto& stored : _denseData) {
				fn_(stored);
			}
		}

		void forEachMut(FnRef<void(ref<BaseModelType_>)> fn_) override {
			for (auto& stored : _denseData) {
				fn_(stored);
			}
		}

		template <typename Fn_> requires
			(not std::is_invocable_v<Fn_, ref<BaseModelType_>>) &&
			std::is_invocable_v<Fn_, ref<ModelType_>>
		void forEachMut(Fn_&& fn_) {
			for (auto& stored : _denseData) {
				fn_(stored);
			}
		}
	};
}
