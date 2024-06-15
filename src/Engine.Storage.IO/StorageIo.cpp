#include "StorageIo.hpp"

#include <Engine.Asserts/Todo.hpp>

using namespace hg::engine::storage;
using namespace hg;

StorageIo::StorageIo(ref<Engine> engine_) noexcept :
	_engine(std::addressof(engine_)) {}
