#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>

class ESPMod {
public:
    static ESPMod& instance() {
        static ESPMod mod;
        return mod;
    }

    ESPMod() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    bool load() {
        getSelf().getLogger().info("Minecraft ESP prototype loaded");
        return true;
    }

    bool enable() {
        return pl::modmenu::ModuleBuilder("minecraft_esp.overlay", "3D ESP")
            .modId(getSelf().getId())
            .description("ESP prototype: rendering is not implemented yet.")
            .defaultEnabled(false)
            .registerModule();
    }

    bool disable() {
        return true;
    }

private:
    ll::mod::NativeMod& mSelf;
};

PL_REGISTER_MOD(ESPMod, ESPMod::instance())
