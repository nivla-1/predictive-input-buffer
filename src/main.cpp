#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

using namespace geode::prelude;

namespace {
    bool s_inLevel = false;
    bool s_inEditor = false;

    class $modify(BufferedGameLayer, GJBaseGameLayer) {
        void handleButton(bool down, int button, bool player2) {
            // Fail open: until a real wave/trajectory predictor exists, never
            // delay or swallow player input. The previous prototype queued
            // presses without predicting whether delaying them was safe.
            GJBaseGameLayer::handleButton(down, button, player2);
        }
    };

    class $modify(BufferedPlayLayer, PlayLayer) {
        bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
            if (!PlayLayer::init(level, useReplay, dontCreateObjects))
                return false;
            s_inLevel = true;
            s_inEditor = false;
            return true;
        }

        void onQuit() {
            s_inLevel = false;
            PlayLayer::onQuit();
        }
    };

    class $modify(BufferedEditorLayer, LevelEditorLayer) {
        bool init(GJGameLevel* level, bool unknown) {
            s_inEditor = true;
            s_inLevel = false;
            return LevelEditorLayer::init(level, unknown);
        }
    };
}

$on_mod(Loaded) {
    log::info("Predictive Input Buffer loaded in safe pass-through mode; input buffering is temporarily disabled.");
}
