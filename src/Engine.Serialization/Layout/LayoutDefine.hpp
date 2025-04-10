#pragma once

#include <span>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/AutoArray.hpp>

#include "DataLayoutBase.hpp"
#include "__fwd.hpp"
#include "../Archive/__fwd.hpp"

namespace hg::engine::serialization::layout {
	class LayoutDefine {
	public:
		using this_type = LayoutDefine;

	protected:
		constexpr LayoutDefine(const u64 offset_, const s64 size_) :
			_offset(offset_),
			_size(size_),
			_store(nullptr),
			_load(nullptr),
			_sub() {}

	public:
		~LayoutDefine() noexcept = default;

	private:
	public:
		u64 _offset;
		s64 _size;

	public:
		[[nodiscard]] s64 size() const noexcept {
			return _size;
		}

	private:
	public:
		ref<resource::Archive> (*_store)(
			ref<resource::Archive> archive_,
			cref<LayoutDefine> define_,
			cref<std::span<u8, std::dynamic_extent>> src_
		);

		ref<resource::Archive> (*_load)(
			ref<resource::Archive> archive_,
			cref<LayoutDefine> define_,
			cref<std::span<u8, std::dynamic_extent>> dst_
		);

	private:
	public:
		sptr<DataLayoutBase> _sub;

	public:
		[[nodiscard]] bool hasSubDefines() const noexcept {
			return _sub != nullptr;
		}

		[[nodiscard]] cref<sptr<DataLayoutBase>> getSubDefines() const noexcept {
			return _sub;
		}
	};
}
