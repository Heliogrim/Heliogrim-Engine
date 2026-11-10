#include "BufferFactory.hpp"

using namespace hg::engine::gfx;
using namespace hg;

uptr<BufferFactory> BufferFactory::_instance = nullptr;

BufferBuildPayload::operator bool() const noexcept {
	return (
		byteSize != 0uLL &&
		byteAlign != ~0uLL &&
		memoryProperties.unwrap != 0 &&
		!!vkUsage
	);
}

nmpt<BufferFactory> BufferFactory::get() {
	return _instance.get();
}

void BufferFactory::destroy() {
	_instance.reset();
}

BufferFactory::BufferFactory(cref<sptr<Device>> device_) :
	_device(device_) {}

BufferFactory::~BufferFactory() noexcept = default;