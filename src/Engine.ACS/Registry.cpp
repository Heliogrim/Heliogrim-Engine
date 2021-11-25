#include "Registry.hpp"

using namespace ember::engine::acs;
using namespace ember;

Registry::~Registry() {

    /**
     * Cleanup adaptors
     */
    for (auto& entry : _adapters) {
        delete entry.second;
        entry.second = nullptr;
    }
}

pool_adapter* Registry::get_pool_adapter(cref<component_type_id> typeId_) {
    const auto entry = _adapters.find(typeId_);

    if (entry != _adapters.end()) {
        return entry->second;
    }

    return nullptr;
}
