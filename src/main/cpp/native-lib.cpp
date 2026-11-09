#include "esp_overlay.h"

#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>
#include <pl/memory/Signature.hpp>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <cstdint>
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

struct SignatureScanResult {
    std::size_t matches{};
    std::uintptr_t first{};
    std::uintptr_t firstFileOffset{};
};

constexpr std::uintptr_t kExpectedTaaFileOffset = 0x1290c4a8u;

SignatureScanResult countExecutableSignatureMatches() {
    constexpr std::array<std::uint8_t, 40> pattern{
        0xFF, 0x83, 0x07, 0xD1, 0xFD, 0x7B, 0x18, 0xA9,
        0xFC, 0x6F, 0x19, 0xA9, 0xFA, 0x67, 0x1A, 0xA9,
        0xF8, 0x5F, 0x1B, 0xA9, 0xF6, 0x57, 0x1C, 0xA9,
        0xF4, 0x4F, 0x1D, 0xA9, 0xFD, 0x03, 0x06, 0x91,
        0x48, 0xD0, 0x3B, 0xD5, 0x39, 0x00, 0x01, 0xD0
    };

    SignatureScanResult result{};
    FILE* maps = std::fopen("/proc/self/maps", "r");
    if (!maps) return result;

    char line[4096];
    while (std::fgets(line, sizeof(line), maps)) {
        unsigned long long start = 0;
        unsigned long long end = 0;
        unsigned long long fileOffset = 0;
        char permissions[5]{};
        if (std::strstr(line, "libminecraftpe.so") == nullptr ||
            std::sscanf(line, "%llx-%llx %4s %llx", &start, &end, permissions, &fileOffset) != 4 ||
            permissions[0] != 'r' || permissions[2] != 'x' ||
            end <= start || end - start < pattern.size()) {
            continue;
        }

        const auto* bytes = reinterpret_cast<const std::uint8_t*>(
            static_cast<std::uintptr_t>(start));
        const std::size_t length = static_cast<std::size_t>(end - start);
        for (std::size_t offset = 0; offset <= length - pattern.size(); ++offset) {
            if (std::memcmp(bytes + offset, pattern.data(), pattern.size()) == 0) {
                ++result.matches;
                if (result.first == 0) {
                    result.first = static_cast<std::uintptr_t>(start) + offset;
                    result.firstFileOffset =
                        static_cast<std::uintptr_t>(fileOffset) + offset;
                }
            }
        }
    }
    std::fclose(maps);
    return result;
}

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
            const auto scan = countExecutableSignatureMatches();
            mSelf->getLogger().info(
                "TAA signature resolver candidate: {:#x}; independent executable-map matches: {}; first match: {:#x}; file offset: {:#x}; expected offset: {:#x}",
                taaCandidate, scan.matches, scan.first, scan.firstFileOffset,
                kExpectedTaaFileOffset);
            if (scan.matches == 1 && scan.first == taaCandidate &&
                scan.firstFileOffset == kExpectedTaaFileOffset) {
                mSelf->getLogger().info(
                    "TAA signature is unique and matches expected file offset; hook remains disabled pending ABI/lifetime validation");
            } else {
                mSelf->getLogger().warn(
                    "TAA signature count/address/file-offset check failed or was inconclusive; hook remains disabled");
            }
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
