#define DEBUG_CPP

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/stdafx.h>

#include <Ember/Ember.hpp>
#include <Ember.Main/Ember.Main.hpp>

#include "Boot/AssetInit.hpp"

using namespace ember;

void ember_main_entry() {

    execute(editor::boot::initAssets);
}
