#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>
#include <deque>

using namespace geode::prelude;

namespace {
    struct QueuedInput {
        bool down;
        int button;
        bool player2;
        int ticksLeft;
    };

    std::deque<QueuedInput> s_queue;
    bool s_replaying = false;
    int s_bufferTicks = 1;

    int clampTicks(int value) {
        return std::max(1, std::min(16, value));
    }

    void loadBufferSetting() {
        s_bufferTicks = clampTicks(
            static_cast<int>(Mod::get()->getSavedValue<int64_t>("buffer-ticks", 1))
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

    class $modify(BufferedGameLayer, GJBaseGameLayer) {
        void handleButton(bool down, int button, bool player2) {
            if (s_replaying || !PlayLayer::get()) {
                GJBaseGameLayer::handleButton(down, button, player2);
                return;
            }
            s_queue.push_back({down, button, player2, s_bufferTicks});
        }
    };

    class $modify(BufferedPlayLayer, PlayLayer) {
        void update(float dt) {
            PlayLayer::update(dt);
            if (this->m_isPaused || s_queue.empty())
                return;

            for (auto& event : s_queue) {
                if (event.ticksLeft > 0)
                    --event.ticksLeft;
            }

            while (!s_queue.empty() && s_queue.front().ticksLeft <= 0) {
                auto event = s_queue.front();
                s_queue.pop_front();

                s_replaying = true;
                if (auto layer = GJBaseGameLayer::get())
                    layer->handleButton(event.down, event.button, event.player2);
                s_replaying = false;
            }
        }

        void onQuit() {
            s_queue.clear();
            PlayLayer::onQuit();
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
