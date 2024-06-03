#pragma once
#include <Engine.Common/Managed/Rc.hpp>

namespace hg::engine::storage {
	class __declspec(novtable) IStorage :
		public ArcFromThis<IStorage> {
	public:
		using this_type = IStorage;

	protected:
		IStorage() noexcept = default;

	public:
		constexpr virtual ~IStorage() = default;

	public:
		[[nodiscard]] virtual bool isReadable() const noexcept = 0;

		[[nodiscard]] virtual bool isWriteable() const noexcept = 0;

		[[nodiscard]] virtual bool isRandomAccess() const noexcept = 0;

		[[nodiscard]] virtual bool isRandomReadable() const noexcept = 0;

		[[nodiscard]] virtual bool isRandomWritable() const noexcept = 0;
	};
}
