#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/AssociativeKey.hpp>
#include <Engine.Common/Collection/StableUnorderedMap.hpp>
#include <Engine.Reflow/Style/StyleSheet.hpp>

namespace ember::editor::ui {

    class Style {
    public:
        using this_type = Style;

        using key_type = AssocKey<string>;

    public:
        [[nodiscard]] static const ptr<this_type> get();

        static const ptr<this_type> make();

        static void destroy();

    protected:
        /**
         * Singleton Instance
         */
        static uptr<Style> _instance;

        Style();

    public:
        ~Style() noexcept;

    protected:
        void setup();

    private:
        StableUnorderedMap<key_type, sptr<engine::reflow::StyleSheet>> _styles;

    public:
        [[nodiscard]] sptr<engine::reflow::StyleSheet> getStyleSheet(cref<key_type> key_) const noexcept;

    public:
        inline static const key_type DefaultKey { key_type::from("Default") };
        inline static const key_type NeutralKey { key_type::from("Neutral") };

        inline static const key_type ScrollBoxKey { key_type::from("ScrollBox") };
        inline static const key_type AdoptFlexBoxKey { key_type::from("AdoptFlexBox") };
        inline static const key_type ButtonKey { key_type::from("Button") };
        inline static const key_type TextButtonKey { key_type::from("TextButton") };

        inline static const key_type Icon32Key { key_type::from("Icon32") };
        inline static const key_type Icon72Key { key_type::from("Icon72") };
        inline static const key_type Icon96Key { key_type::from("Icon96") };
        inline static const key_type Icon128Key { key_type::from("Icon128") };

        inline static const key_type TitleKey { key_type::from("Title") };
        inline static const key_type TitleSmallKey { key_type::from("TitleSmall") };

        /**/

        inline static const key_type ButtonRaisedKey { key_type::from("Button::Raised") };
        inline static const key_type TitleRaisedKey { key_type::from("Title::Raised") };

        /**/

        inline static const key_type BreadcrumbKey { key_type::from("Breadcrumb") };
    };

}
