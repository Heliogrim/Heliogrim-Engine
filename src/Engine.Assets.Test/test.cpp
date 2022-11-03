#include "pch.h"

#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Assets/Database/AssetDatabaseQuery.hpp>

using namespace ember::engine::assets;
using namespace ember;

TEST(__DummyTest__, Exists) {
    EXPECT_TRUE(true);
}

namespace AssetModule {

    namespace AssetDatabase {

        TEST(Database, Default) {
            //
            engine::assets::AssetDatabase db {};

            //
            // Note: Implicit invoke: db.tidy();
        }

        //
        class PayloadLayout : DataLayout {};

        struct PayloadType : Data<PayloadLayout> {
            float pos[4];

            inline static constexpr asset_type_id typeId { "PayloadType"_typeId };

            constexpr asset_type_id getTypeId() const noexcept {
                return PayloadType::typeId;
            }

            PayloadType(const asset_guid& guid_) :
                Data(guid_, typeId) {}
        };

        TEST(Database, Insert) {
            //
            engine::assets::AssetDatabase db {};

            //
            auto guid = generate_asset_guid();
            auto* payload { EmberObject::create<PayloadType>(guid) };

            //
            EXPECT_FALSE(db.has(guid));

            EXPECT_TRUE(db.insert(guid, payload));

            EXPECT_TRUE(db.has(guid));

            //
            db.tidy();
        }

        TEST(Database, Select) {
            //
            engine::assets::AssetDatabase db {};

            //
            auto guid = generate_asset_guid();
            auto* payload { EmberObject::create<PayloadType>(guid) };

            //
            EXPECT_FALSE(db.has(guid));

            EXPECT_TRUE(db.insert(guid, payload));
            EXPECT_EQ(db[guid], payload);

            EXPECT_TRUE(db.has(guid));

            //
            db.tidy();
        }

        TEST(Database, Erase) {
            //
            engine::assets::AssetDatabase db {};

            //
            auto guid = generate_asset_guid();
            auto* payload { EmberObject::create<PayloadType>(guid) };

            //
            EXPECT_FALSE(db.has(guid));

            EXPECT_TRUE(db.insert(guid, payload));
            EXPECT_TRUE(db.has(guid));

            EXPECT_EQ(db.remove(guid), payload);
            EXPECT_FALSE(db.has(guid));

            //
            EmberObject::destroy(_STD move(payload));
            db.tidy();
        }

        TEST(DatabaseQuery, Default) {
            //
            engine::assets::AssetDatabase db {};

            //
            auto guid = generate_asset_guid();

            //
            engine::assets::AssetDatabaseQuery query = db.query(guid);

            //
            db.tidy();
        }

        TEST(DatabaseQuery, Insert) {
            //
            engine::assets::AssetDatabase db {};

            //
            auto guid = generate_asset_guid();
            auto* payload { EmberObject::create<PayloadType>(guid) };

            //
            engine::assets::AssetDatabaseQuery query = db.query(guid);

            //
            EXPECT_FALSE(query.exists());

            EXPECT_TRUE(query.insert<PayloadType>(payload));

            EXPECT_TRUE(query.exists());

            //
            db.tidy();
        }

        TEST(DatabaseQuery, Select) {
            //
            engine::assets::AssetDatabase db {};

            //
            auto guid = generate_asset_guid();
            auto* payload { EmberObject::create<PayloadType>(guid) };

            //
            engine::assets::AssetDatabaseQuery query = db.query(guid);

            //
            EXPECT_FALSE(query.exists());

            EXPECT_TRUE(query.insert<PayloadType>(payload));
            EXPECT_EQ(query.get(), payload);

            EXPECT_TRUE(query.exists());

            //
            db.tidy();
        }

        TEST(DatabaseQuery, Erase) {
            //
            engine::assets::AssetDatabase db {};

            //
            auto guid = generate_asset_guid();
            auto* payload { EmberObject::create<PayloadType>(guid) };

            //
            engine::assets::AssetDatabaseQuery query = db.query(guid);

            //
            EXPECT_FALSE(query.exists());

            EXPECT_TRUE(query.insert<PayloadType>(payload));
            EXPECT_TRUE(query.exists());

            EXPECT_EQ(query.remove(), payload);

            //
            EmberObject::destroy(_STD move(payload));
            db.tidy();
        }

    }

}
