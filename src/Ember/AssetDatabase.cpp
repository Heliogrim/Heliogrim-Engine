#include "AssetDatabase.hpp"

using namespace ember;

AssetDatabase::AssetDatabase(managed<void> internal_) :
    _internal(internal_) {}

AssetDatabase::~AssetDatabase() = default;
