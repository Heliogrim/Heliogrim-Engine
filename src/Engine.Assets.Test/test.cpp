#include "pch.h"

#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Assets/Database/AssetDatabaseQuery.hpp>

using namespace ember::engine::asset;
using namespace ember;

TEST(__DummyTest__, Exists) {
    EXPECT_TRUE(true);
}

namespace AssetModule {

    namespace AssetDatabase {

        TEST(Database, Default) {
            //
            engine::asset::AssetDatabase db {};

            //
            // Note: Implicit invoke: db.tidy();
        }

        //
        struct PayloadType : Asset {
            float pos[4];

            static constexpr asset_type_id typeId = { "PayloadType"_typeId };

            constexpr asset_type_id getTypeId() const noexcept {
                return PayloadType::typeId;
            }

            PayloadType(const asset_guid& guid_) :
                Asset(guid_) {}
        };

        TEST(Database, Insert) {
            //
            engine::asset::AssetDatabase db {};

            //
            auto guid = generate_entity_guid();
            PayloadType payload { guid };

            //
            EXPECT_FALSE(db.has(guid));

            EXPECT_TRUE(db.insert(guid, &payload));

            EXPECT_TRUE(db.has(guid));

            //
            db.tidy();
        }

        TEST(Database, Select) {
            //
            engine::asset::AssetDatabase db {};

            //
            auto guid = generate_entity_guid();
            PayloadType payload { guid };

            //
            EXPECT_FALSE(db.has(guid));

            EXPECT_TRUE(db.insert(guid, &payload));
            EXPECT_EQ(db[guid], &payload);

            EXPECT_TRUE(db.has(guid));

            //
            db.tidy();
        }

        TEST(Database, Erase) {
            //
            engine::asset::AssetDatabase db {};

            //
            auto guid = generate_entity_guid();
            PayloadType payload { guid };

            //
            EXPECT_FALSE(db.has(guid));

            EXPECT_TRUE(db.insert(guid, &payload));
            EXPECT_TRUE(db.has(guid));

            EXPECT_EQ(db.remove(guid), &payload);
            EXPECT_FALSE(db.has(guid));

            //
            db.tidy();
        }

        TEST(DatabaseQuery, Default) {
            //
            engine::asset::AssetDatabase db {};

            //
            auto guid = generate_entity_guid();

            //
            engine::asset::AssetDatabaseQuery query = db.query(guid);

            //
            db.tidy();
        }

        TEST(DatabaseQuery, Insert) {
            //
            engine::asset::AssetDatabase db {};

            //
            auto guid = generate_entity_guid();
            PayloadType payload { guid };

            //
            engine::asset::AssetDatabaseQuery query = db.query(guid);

            //
            EXPECT_FALSE(query.exists());

            EXPECT_TRUE(query.insert(&payload));

            EXPECT_TRUE(query.exists());

            //
            db.tidy();
        }

        TEST(DatabaseQuery, Select) {
            //
            engine::asset::AssetDatabase db {};

            //
            auto guid = generate_entity_guid();
            PayloadType payload { guid };

            //
            engine::asset::AssetDatabaseQuery query = db.query(guid);

            //
            EXPECT_FALSE(query.exists());

            EXPECT_TRUE(query.insert(&payload));
            EXPECT_EQ(query.get(), &payload);

            EXPECT_TRUE(query.exists());

            //
            db.tidy();
        }

        TEST(DatabaseQuery, Erase) {
            //
            engine::asset::AssetDatabase db {};

            //
            auto guid = generate_entity_guid();
            PayloadType payload { guid };

            //
            engine::asset::AssetDatabaseQuery query = db.query(guid);

            //
            EXPECT_FALSE(query.exists());

            EXPECT_TRUE(query.insert(&payload));
            EXPECT_TRUE(query.exists());

            EXPECT_EQ(query.remove(), &payload);

            //
            db.tidy();
        }

    }

}
