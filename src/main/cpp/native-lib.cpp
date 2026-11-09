#include "esp_overlay.h"

#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>
#include <pl/memory/Signature.hpp>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <filesystem>
#include <span>
#include <vector>
#include <string>
#include <string_view>

namespace {
constexpr std::string_view kModuleId = "minecraft_esp.overlay";

std::atomic_bool g_moduleEnabled{false};
std::atomic_bool g_playersEnabled{true};
std::atomic_bool g_itemsEnabled{true};
std::atomic_bool g_moduleRegistered{false};

bool parse_bool(std::string_view value, bool fallback) {
    if (value == "true" || value == "1" || value == "on") return true;
    if (value == "false" || value == "0" || value == "off") return false;
    return fallback;
}

class MinecraftEspMod {
public:
    static MinecraftEspMod& instance() {
        static MinecraftEspMod mod;
        return mod;
    }

    bool load() {
        // PLGetModRegistration() may construct this singleton before a lifecycle
        // scope exists. Resolve NativeMod::current() only inside load(), where
        // Preloader guarantees the current-mod context.
        mSelf = ll::mod::NativeMod::current();
        if (!mSelf) return false;

        std::error_code error;
        std::filesystem::create_directories(mSelf->getConfigDir(), error);
        if (error) {
            mSelf->getLogger().warn("Could not create config directory: {}", error.message());
        }
        mSelf->getLogger().info("Minecraft ESP native module loaded (render hooks pending)");
        return true;
    }

    bool enable() {
        if (!mSelf) return false;

        // Diagnostic only: resolve the statically unique TAA callback candidate
        // in the live game module. This deliberately does not install a hook.
        constexpr std::string_view kTaaCallbackSignature =
            "FF 83 07 D1 FD 7B 18 A9 FC 6F 19 A9 FA 67 1A A9 "
            "F8 5F 1B A9 F6 57 1C A9 F4 4F 1D A9 FD 03 06 91 "
            "48 D0 3B D5 39 00 01 D0";
        const auto taaCandidate = pl::memory::resolveSignature(
            kTaaCallbackSignature, "libminecraftpe.so");
        if (taaCandidate) {
            mSelf->getLogger().info(
                "TAA callback signature candidate resolved at {:#x}; hook remains disabled",
                taaCandidate);
        } else {
            mSelf->getLogger().warn(
                "TAA callback signature candidate was not found in libminecraftpe.so");
        }

        pl::modmenu::ModuleBuilder builder(std::string(kModuleId), "3D ESP");
        const bool registered = builder
            .modId(mSelf->getId())
            .description("3D entity outlines. Runtime entity and camera integration is not active yet.")
            .defaultEnabled(false)
            .config("players", "Players", pl::modmenu::ConfigType::Toggle, "true")
            .config("dropped_items", "Dropped items", pl::modmenu::ConfigType::Toggle, "true")
            .onToggle([](std::string_view, bool enabled) {
                g_moduleEnabled.store(enabled, std::memory_order_release);
                if (!enabled) esp_overlay_clear();
            })
            .onConfigChanged([](std::string_view, std::string_view key,
                                std::string_view value) {
                if (key == "players") {
                    g_playersEnabled.store(parse_bool(value, true), std::memory_order_release);
                } else if (key == "dropped_items") {
                    g_itemsEnabled.store(parse_bool(value, true), std::memory_order_release);
                }
            })
            .registerModule();

        g_moduleRegistered.store(registered, std::memory_order_release);
        if (!registered) {
            mSelf->getLogger().error("Failed to register 3D ESP Mod Menu module");
            return false;
        }
        mSelf->getLogger().info("3D ESP toggle registered in LeviLauncher Mod Menu");
        return true;
    }

    bool disable() {
        g_moduleEnabled.store(false, std::memory_order_release);
        esp_overlay_clear();
        if (g_moduleRegistered.exchange(false, std::memory_order_acq_rel)) {
            pl::modmenu::unregisterModule(kModuleId);
        }
        return true;
    }

    bool unload() {
        disable();
        if (mSelf) mSelf->getLogger().info("Minecraft ESP native module unloaded");
        return true;
    }

private:
    ll::mod::NativeMod* mSelf{};
};
}

bool esp_overlay_submit_frame(const EspScreenSegment* playerSegments,
                              std::size_t playerCount,
                              const EspScreenSegment* itemSegments,
                              std::size_t itemCount,
                              float thickness) {
    if (!g_moduleRegistered.load(std::memory_order_acquire) ||
        !g_moduleEnabled.load(std::memory_order_acquire) ||
        (playerCount && !playerSegments) || (itemCount && !itemSegments) ||
        !std::isfinite(thickness) || thickness <= 0.0f) {
        return false;
    }

    constexpr std::size_t kMaxCommands = 4096;
    std::vector<pl::modmenu::DrawCommand> commands;
    const std::size_t reserveCount =
        playerCount >= kMaxCommands || itemCount >= kMaxCommands ||
        playerCount > kMaxCommands - std::min(itemCount, kMaxCommands)
            ? kMaxCommands
            : playerCount + itemCount;
    commands.reserve(reserveCount);

    const auto append = [&](const EspScreenSegment* segments,
                            std::size_t count,
                            bool enabled,
                            std::uint32_t color) {
        if (!enabled || !segments || commands.size() >= kMaxCommands) return;
        const std::size_t available = kMaxCommands - commands.size();
        const std::size_t limit = std::min(count, available);
        for (std::size_t i = 0; i < limit; ++i) {
            const auto& segment = segments[i];
            if (!segment.valid ||
                !std::isfinite(segment.x1) || !std::isfinite(segment.y1) ||
                !std::isfinite(segment.x2) || !std::isfinite(segment.y2)) {
                continue;
            }
            commands.push_back({
                .type = pl::modmenu::DrawCommandType::Line,
                .x = segment.x1,
                .y = segment.y1,
                .w = segment.x2 - segment.x1,
                .h = segment.y2 - segment.y1,
                .x3 = 0.0f,
                .y3 = 0.0f,
                .color = color,
                .size = thickness,
                .text = {},
                .fontId = {},
                .imageId = {}
            });
        }
    };

    append(playerSegments, playerCount,
           g_playersEnabled.load(std::memory_order_acquire), 0xFFFF3030u);
    append(itemSegments, itemCount,
           g_itemsEnabled.load(std::memory_order_acquire), 0xFF30E060u);

    pl::modmenu::submitDrawCommands(kModuleId, commands);
    return true;
}

void esp_overlay_clear() {
    if (!g_moduleRegistered.load(std::memory_order_acquire)) return;
    pl::modmenu::submitDrawCommands(
        kModuleId, std::span<const pl::modmenu::DrawCommand>{});
}

PL_REGISTER_MOD(MinecraftEspMod, MinecraftEspMod::instance())
