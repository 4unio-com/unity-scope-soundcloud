#include <scope/localization.h>
#include <scope/preview.h>

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/VariantBuilder.h>

#include <iostream>

namespace sc = unity::scopes;

using namespace std;
using namespace scope;

Preview::Preview(const sc::Result &result, const sc::ActionMetadata &metadata) :
    sc::PreviewQueryBase(result, metadata) {
}

void Preview::cancelled() {
}

void Preview::run(sc::PreviewReplyProxy const& reply) {
    auto const res = result();

    // Support three different column layouts
    sc::ColumnLayout layout1col(1), layout2col(2), layout3col(3);

    // Single column layout
    layout1col.add_column( { "header", "art", "statistics", "tracks", "actions", "description" });

    // Register the layouts we just created
    reply->register_layout( { layout1col }); //, layout2col, layout3col

    sc::PreviewWidget header("header", "header");
    header.add_attribute_mapping("title", "title");
    header.add_attribute_mapping("subtitle", "username");

    sc::PreviewWidget art("art", "image");
    art.add_attribute_mapping("source", "art");

    sc::PreviewWidget statistics("statistics", "header");
    statistics.add_attribute_value("title", sc::Variant(res["playback-count"].get_string() + "   " + res["favoritings-count"].get_string()));

    sc::PreviewWidget tracks("tracks", "audio");
    {
        if (res["streamable"].get_bool()) {
            sc::VariantBuilder builder;
            builder.add_tuple({
                    {"title", sc::Variant(res.title())},
                    {"source", sc::Variant(res["stream-url"])},
                    {"length", res["duration"]}
                });
            tracks.add_attribute_value("tracks", builder.end());
        }
    }

    sc::PreviewWidget actions("actions", "actions");
    {
        string purchase_url = res["purchase-url"].get_string();
        if (!purchase_url.empty()) {
            sc::VariantBuilder builder;
            builder.add_tuple({
                    {"id", sc::Variant("buy")},
                    {"label", sc::Variant(_("Buy"))},
                    {"uri", sc::Variant(purchase_url)}
                });
            actions.add_attribute_value("actions", builder.end());
        }
        string video_url = res["video-url"].get_string();
        if (!video_url.empty()) {
            sc::VariantBuilder builder;
            builder.add_tuple({
                    {"id", sc::Variant("video")},
                    {"label", sc::Variant(_("Watch video"))},
                    {"uri", sc::Variant(video_url)}
                });
            actions.add_attribute_value("actions", builder.end());
        }
        {
            sc::VariantBuilder builder;
            builder.add_tuple({
                    {"id", sc::Variant("play")},
                    {"label", sc::Variant(_("Play in browser"))}
                });
            actions.add_attribute_value("actions", builder.end());
        }
    }

    sc::PreviewWidget description("description", "text");
    description.add_attribute_mapping("text", "description");

    reply->push( { header, art, statistics, tracks, actions, description });
}
