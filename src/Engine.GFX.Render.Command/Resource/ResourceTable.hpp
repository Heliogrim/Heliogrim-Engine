#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

#include "Resource.hpp"

namespace hg::engine::render {
	class __declspec(novtable) ResourceTable {
	public:
		using this_type = ResourceTable;

		using SymbolId = ::hg::engine::accel::lang::SymbolId;
		using Resource = ::hg::engine::render::Resource;

	public:
		constexpr ResourceTable() noexcept = default;

		constexpr ResourceTable(const this_type&) noexcept = default;

		constexpr ResourceTable(this_type&&) noexcept = default;

		constexpr virtual ~ResourceTable() noexcept = default;

	public:
		constexpr ref<this_type> operator=(cref<this_type>) noexcept = default;

		constexpr ref<this_type> operator=(mref<this_type>) noexcept = default;

	public:
		virtual void bind(cref<SymbolId> symbolId_, mref<Resource> resource_) = 0;
	};
}
