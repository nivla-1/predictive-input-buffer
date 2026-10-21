#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EffectGameObject.hpp>

#include <algorithm>

using namespace geode::prelude;

namespace {
    struct PendingInput {
        bool active = false;
        bool down = false;
        int button = 0;
        bool player2 = false;
        int startTick = 0;
    };

    PendingInput s_player1Pending;
    PendingInput s_player2Pending;
    bool s_replaying = false;
    bool s_inLevel = false;
    bool s_inEditor = false;
    int s_tick = 0;
    int s_bufferTicks = 4;

    int clampTicks(int value) {
        return std::max(1, std::min(16, value));
    }

    PendingInput& pendingFor(bool player2) {
        return player2 ? s_player2Pending : s_player1Pending;
    }

    void clearPending() {
        s_player1Pending = {};
        s_player2Pending = {};
    }

    void loadBufferSetting() {
        s_bufferTicks = clampTicks(
            static_cast<int>(Mod::get()->getSavedValue<int64_t>("buffer-ticks", 4))
        );
        Mod::get()->setSavedValue<int64_t>("buffer-ticks", s_bufferTicks);
    }

    void cycleBuffer() {
        s_bufferTicks = (s_bufferTicks % 16) + 1;
        Mod::get()->setSavedValue<int64_t>("buffer-ticks", s_bufferTicks);
        Notification::create(
            fmt::format("Input buffer: {} tick{}", s_bufferTicks,
                s_bufferTicks == 1 ? "" : "s"),
            1.2f
        )->show();
        log::info("Input buffer set to {} ticks", s_bufferTicks);
    }

    void replayPending(GJBaseGameLayer* layer, PendingInput& pending) {
        if (!layer || !pending.active)
            return;

        auto event = pending;
        pending = {};

        s_replaying = true;
        layer->handleButton(event.down, event.button, event.player2);
        s_replaying = false;
    }

    class $modify(BufferedGameLayer, GJBaseGameLayer) {
        void handleButton(bool down, int button, bool player2) {
            if (s_replaying || !s_inLevel || s_inEditor || !PlayLayer::get()) {
                GJBaseGameLayer::handleButton(down, button, player2);
                return;
            }

            auto& pending = pendingFor(player2);

            // Never swallow a second event while an input is pending; pass it
            // through normally so a held button can still be released.
            if (pending.active) {
                GJBaseGameLayer::handleButton(down, button, player2);
                if (!down)
                    pending = {};
                return;
            }

            // Buffer presses only. Releases remain immediate to avoid making
            // the player's hold duration unexpectedly longer.
            if (!down) {
                GJBaseGameLayer::handleButton(down, button, player2);
                return;
            }

            pending = {true, down, button, player2, s_tick};
        }

        void collisionCheckObjects(PlayerObject* player, gd::vector<GameObject*>* objects, int objectsCount, float dt) {
            // This hook uses the Geometry Dash 2.2081 signature. The current
            // prototype only uses the list to notice activatable objects; this
            // is not a wave collision predictor.
            if (s_inLevel && !s_inEditor && player && objects && objectsCount > 0) {
                auto* playLayer = PlayLayer::get();
                bool player2 = playLayer && player == playLayer->m_player2;
                auto& pending = pendingFor(player2);
                if (pending.active) {
                    auto count = std::min(objectsCount, static_cast<int>(objects->size()));
                    for (int i = 0; i < count; ++i) {
                        auto* effect = typeinfo_cast<EffectGameObject*>((*objects)[i]);
                        if (!effect)
                            continue;
                        if (!GJBaseGameLayer::canBeActivatedByPlayer(player, effect))
                            continue;
                        replayPending(this, pending);
                        break;
                    }
                }
            }
            GJBaseGameLayer::collisionCheckObjects(player, objects, objectsCount, dt);
        }
    };

    class $modify(BufferedPlayLayer, PlayLayer) {
        bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
            if (!PlayLayer::init(level, useReplay, dontCreateObjects))
                return false;
            s_inLevel = true;
            s_inEditor = false;
            s_tick = 0;
            clearPending();
            return true;
        }

        void update(float dt) {
            PlayLayer::update(dt);
            if (this->m_isPaused || !s_inLevel)
                return;

            ++s_tick;
            auto* layer = static_cast<GJBaseGameLayer*>(this);
            if (s_player1Pending.active && s_tick - s_player1Pending.startTick >= s_bufferTicks)
                replayPending(layer, s_player1Pending);
            if (s_player2Pending.active && s_tick - s_player2Pending.startTick >= s_bufferTicks)
                replayPending(layer, s_player2Pending);
        }

        void destroyPlayer(PlayerObject* player, GameObject* object) {
            clearPending();
            PlayLayer::destroyPlayer(player, object);
        }

        void onQuit() {
            s_inLevel = false;
            clearPending();
            PlayLayer::onQuit();
        }

        void resetLevel() {
            clearPending();
            s_tick = 0;
            PlayLayer::resetLevel();
        }
    };

    class $modify(BufferedPlayerObject, PlayerObject) {
        void incrementJumps() {
            auto* playLayer = PlayLayer::get();
            if (playLayer) {
                bool player2 = this == playLayer->m_player2;
                pendingFor(player2) = {};
            }
            PlayerObject::incrementJumps();
        }

        void ringJump(RingObject* ring, bool unknown) {
            auto* playLayer = PlayLayer::get();
            if (playLayer) {
                bool player2 = this == playLayer->m_player2;
                pendingFor(player2) = {};
            }
            PlayerObject::ringJump(ring, unknown);
        }
    };

    class $modify(BufferedEditorLayer, LevelEditorLayer) {
        bool init(GJGameLevel* level, bool unknown) {
            s_inEditor = true;
            s_inLevel = false;
            clearPending();
            return LevelEditorLayer::init(level, unknown);
        }
    };
}

$on_mod(Loaded) {
    loadBufferSetting();
    listenForKeybindSettingPresses(
        "cycle-buffer",
        [](Keybind const&, bool down, bool repeat, double) {
            if (down && !repeat)
                cycleBuffer();
        }
    );
}
