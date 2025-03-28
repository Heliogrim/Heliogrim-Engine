#pragma once

#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflow.Attributes/Data/DataAttribute.hpp>

#include "Widget/Widget.hpp"

namespace hg::engine::reflow {
	class DataWatcher final {
	public:
		DataWatcher() = default;

		DataWatcher(ref<const DataWatcher>) = delete;

		DataWatcher(mref<DataWatcher>) = delete;

		~DataWatcher() = default;

	public:
		void tick();

	private:
		DenseMap<std::uintptr_t, std::function<void()>> _observers;

	public:
		template <typename DataType_>
		void detach(_Inout_ ref<DataAttribute<DataType_>> attribute_) {
			_observers.erase(reinterpret_cast<std::uintptr_t>(std::addressof(attribute_)));
			attribute_._queryUnsubscribe = nullptr;
		}

		template <typename DataType_>
		void attach(nmpt<Widget> widget_, _Inout_ ref<DataAttribute<DataType_>> attribute_) {
			_observers.insert(
				std::make_pair(
					reinterpret_cast<std::uintptr_t>(std::addressof(attribute_)),
					[attribute = std::addressof(attribute_), widget = widget_]() {
						if (attribute->update(attribute->_query())) {
							widget->markAsPending(true, true);
						}
					}
				)
			);
			attribute_._queryUnsubscribe = [this, attribute = std::addressof(attribute_)]() { this->detach(*attribute); };
		}
	};
}
