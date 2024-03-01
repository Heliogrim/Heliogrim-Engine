#include "pch.h"

#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.System/__fwd.hpp>

using namespace hg::engine::assets;
using namespace hg;

TEST(__DummyTest__, Exists) {
    EXPECT_TRUE(true);
}

namespace AssetModule {

    namespace AssetDatabase {

        TEST(Database, Default) {
        }

        TEST(Database, Insert) {
        }

        TEST(Database, Select) {
        }

        TEST(Database, Erase) {
        }

        TEST(DatabaseQuery, Default) {
        }

        TEST(DatabaseQuery, Insert) {
        }

        TEST(DatabaseQuery, Select) {
        }

        TEST(DatabaseQuery, Erase) {
        }

    }

}
