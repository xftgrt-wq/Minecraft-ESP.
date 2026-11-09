#include "esp_overlay.h"

#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>
#include <pl/memory/Signature.hpp>
#include <pl/memory/Vtable.hpp>

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
#include <link.h>
#include <elf.h>

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

bool isExecutableGameAddress(std::uintptr_t candidate) {
    if (candidate == 0) return false;
    FILE* maps = std::fopen("/proc/self/maps", "r");
    if (!maps) return false;

    bool executable = false;
    char line[4096];
    while (std::fgets(line, sizeof(line), maps)) {
        unsigned long long start = 0;
        unsigned long long end = 0;
        char permissions[5]{};
        if (std::strstr(line, "libminecraftpe.so") == nullptr ||
            std::sscanf(line, "%llx-%llx %4s", &start, &end, permissions) != 3 ||
            permissions[0] != 'r' || permissions[2] != 'x' ||
            end <= start) {
            continue;
        }
        if (candidate >= static_cast<std::uintptr_t>(start) &&
            candidate < static_cast<std::uintptr_t>(end)) {
            executable = true;
            break;
        }
    }
    std::fclose(maps);
    return executable;
}

struct GameModuleIdentity {
    bool found{};
    std::uintptr_t loadBase{};
    char path[256]{};
    char buildId[65]{};
};

std::size_t align_note(std::size_t value) {
    return (value + 3u) & ~std::size_t(3u);
}

int findGameModuleIdentity(dl_phdr_info* info, std::size_t, void* opaque) {
    if (!info || !opaque || !info->dlpi_name ||
        std::strstr(info->dlpi_name, "libminecraftpe.so") == nullptr) {
        return 0;
    }

    auto& identity = *static_cast<GameModuleIdentity*>(opaque);
    identity.found = true;
    identity.loadBase = static_cast<std::uintptr_t>(info->dlpi_addr);
    std::snprintf(identity.path, sizeof(identity.path), "%s", info->dlpi_name);

    for (ElfW(Half) i = 0; i < info->dlpi_phnum; ++i) {
        const auto& header = info->dlpi_phdr[i];
        if (header.p_type != PT_NOTE || header.p_memsz < sizeof(ElfW(Nhdr))) {
            continue;
        }

        const auto* cursor = reinterpret_cast<const std::uint8_t*>(
            identity.loadBase + header.p_vaddr);
        std::size_t remaining = static_cast<std::size_t>(header.p_memsz);
        while (remaining >= sizeof(ElfW(Nhdr))) {
            ElfW(Nhdr) note{};
            std::memcpy(&note, cursor, sizeof(note));
            cursor += sizeof(note);
            remaining -= sizeof(note);

            const std::size_t nameSize = align_note(note.n_namesz);
            const std::size_t descSize = align_note(note.n_descsz);
            if (nameSize > remaining || descSize > remaining - nameSize) {
                break;
            }

            const auto* name = cursor;
            const auto* desc = cursor + nameSize;
            if (note.n_type == NT_GNU_BUILD_ID && note.n_namesz >= 3 &&
                std::memcmp(name, "GNU", 3) == 0 && note.n_descsz > 0) {
                const std::size_t bytes = std::min<std::size_t>(
                    note.n_descsz, (sizeof(identity.buildId) - 1) / 2);
                for (std::size_t j = 0; j < bytes; ++j) {
                    std::snprintf(identity.buildId + j * 2,
                                  sizeof(identity.buildId) - j * 2,
                                  "%02x", desc[j]);
                }
                return 1;
            }

            const std::size_t consumed = nameSize + descSize;
            cursor += consumed;
            remaining -= consumed;
        }
    }
    return 1;
}

bool parse_bool(std::string_view value, bool fallback) {
    if (value == "true" || value == "1" || value == "on") return true;
    if (value == "false" || value == "0" || value == "off") return false;
    return fallback;
}

void submitDiagnosticHud() {
    if (!g_moduleRegistered.load(std::memory_order_acquire) ||
        !g_moduleEnabled.load(std::memory_order_acquire)) {
        return;
    }

    // The pinned ModMenu SDK does not expose HUD surface dimensions.
    // Use conservative fixed coordinates for this bridge-only diagnostic.
    const float panelWidth = 280.0f;
    const float panelHeight = 56.0f;
    const float x = 24.0f;
    const float y = 96.0f;

    const std::array<pl::modmenu::DrawCommand, 3> commands{{
        {
            .type = pl::modmenu::DrawCommandType::RectFilled,
            .x = x,
            .y = y,
            .w = panelWidth,
            .h = panelHeight,
            .x3 = 0.0f,
            .y3 = 0.0f,
            .color = 0xF0000000u,
            .size = 0.0f,
            .text = {},
            .fontId = {},
            .imageId = {}
        },
        {
            .type = pl::modmenu::DrawCommandType::Rect,
            .x = x,
            .y = y,
            .w = panelWidth,
            .h = panelHeight,
            .x3 = 0.0f,
            .y3 = 0.0f,
            .color = 0xFFFF3030u,
            .size = 3.0f,
            .text = {},
            .fontId = {},
            .imageId = {}
        },
        {
            .type = pl::modmenu::DrawCommandType::Text,
            .x = x,
            .y = y,
            .w = panelWidth,
            .h = panelHeight,
            .x3 = 0.0f,
            .y3 = 0.0f,
            .color = 0xFFFFFFFFu,
            .size = 24.0f,
            .text = "ESP HUD TEST - BUILD 124",
            .fontId = {},
            .imageId = {}
        }
    }};
    pl::modmenu::submitDrawCommands(kModuleId, commands);
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

        // Read-only identity diagnostic; no game objects are dereferenced.
        GameModuleIdentity gameIdentity{};
        dl_iterate_phdr(findGameModuleIdentity, &gameIdentity);
        if (gameIdentity.found) {
            mSelf->getLogger().info(
                "Game ELF identity: path='{}' load_base={:#x} build_id='{}'",
                gameIdentity.path, gameIdentity.loadBase,
                gameIdentity.buildId[0] ? gameIdentity.buildId : "missing");
        } else {
            mSelf->getLogger().warn(
                "Game ELF identity: libminecraftpe.so is not present in dl_iterate_phdr");
        }

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

        // Exact-version static research surfaced two RTTI/vtable candidates in a
        // public 1.26.33.1 ARM64 analysis. Treat them as diagnostics only: do not
        // dereference returned game pointers or install hooks based on slot matches.
        const auto localPlayerAccessor = pl::memory::resolveVtableFunction(
            "14ClientInstance", 31, "libminecraftpe.so");
        const auto playerRenderEntry = pl::memory::resolveVtableFunction(
            "18LivePlayerRenderer", 17, "libminecraftpe.so");
        const bool localPlayerAccessorExecutable =
            isExecutableGameAddress(localPlayerAccessor);
        const bool playerRenderEntryExecutable =
            isExecutableGameAddress(playerRenderEntry);
        mSelf->getLogger().info(
            "RTTI diagnostics: ClientInstance slot 31={:#x} executable={}; LivePlayerRenderer slot 17={:#x} executable={}",
            localPlayerAccessor, localPlayerAccessorExecutable,
            playerRenderEntry, playerRenderEntryExecutable);
        if (!localPlayerAccessorExecutable || !playerRenderEntryExecutable) {
            mSelf->getLogger().warn(
                "One or more RTTI/vtable candidates are absent or non-executable; no game hook was installed");
        } else {
            mSelf->getLogger().info(
                "RTTI/vtable candidates resolve to executable game mappings; ABI, slot semantics, and pointer lifetime remain unverified; hooks remain disabled");
        }

        pl::modmenu::ModuleBuilder builder(std::string(kModuleId), "3D ESP");
        const bool registered = builder
            .modId(mSelf->getId())
            .description("3D entity outlines. Runtime entity and camera integration is not active yet.")
            .defaultEnabled(true)
            .config("players", "Players", pl::modmenu::ConfigType::Toggle, "true")
            .config("dropped_items", "Dropped items", pl::modmenu::ConfigType::Toggle, "true")
            .onToggle([this](std::string_view moduleId, bool enabled) {
                g_moduleEnabled.store(enabled, std::memory_order_release);
                mSelf->getLogger().info(
                    "HUD bridge diagnostic: toggle callback module_id='{}' enabled={} registered={}",
                    moduleId, enabled, g_moduleRegistered.load(std::memory_order_acquire));
                if (enabled) {
                    submitDiagnosticHud();
                    mSelf->getLogger().info(
                        "HUD bridge diagnostic: submitted fixed test rect/text commands for '{}'",
                        kModuleId);
                } else {
                    esp_overlay_clear();
                    mSelf->getLogger().info("HUD bridge diagnostic: cleared commands for '{}'", kModuleId);
                }
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
        // Submit once immediately after registration as well as from the
        // toggle callback. This avoids losing the first diagnostic batch if
        // the launcher creates the HUD view slightly later in its lifecycle.
        g_moduleEnabled.store(true, std::memory_order_release);
        submitDiagnosticHud();
        mSelf->getLogger().info(
            "HUD diagnostic submitted after registration at fixed coordinates (24,96)");
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

    // Preserve the fixed-coordinate HUD bridge diagnostic in frame batches.
    if (commands.size() + 2 <= kMaxCommands) {
        commands.push_back({
            .type = pl::modmenu::DrawCommandType::RectFilled,
            .x = 16.0f, .y = 24.0f, .w = 280.0f, .h = 44.0f,
            .x3 = 0.0f, .y3 = 0.0f, .color = 0xE0202020u,
            .size = 0.0f, .text = {}, .fontId = {}, .imageId = {}
        });
        commands.push_back({
            .type = pl::modmenu::DrawCommandType::Text,
            .x = 26.0f, .y = 53.0f, .w = 0.0f, .h = 0.0f,
            .x3 = 0.0f, .y3 = 0.0f, .color = 0xFFFFFFFFu,
            .size = 17.0f, .text = "ESP HUD PIPELINE OK",
            .fontId = {}, .imageId = {}
        });
    }

    pl::modmenu::submitDrawCommands(kModuleId, commands);
    return true;
}

void esp_overlay_clear() {
    if (!g_moduleRegistered.load(std::memory_order_acquire)) return;
    pl::modmenu::submitDrawCommands(
        kModuleId, std::span<const pl::modmenu::DrawCommand>{});
}

PL_REGISTER_MOD(MinecraftEspMod, MinecraftEspMod::instance())
