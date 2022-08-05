#include "main.hpp"
#include "logos.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"

bool HasEnteredSettings;

DEFINE_CONFIG(ModConfig);

using namespace GlobalNamespace;

HMUI::ImageView *SettingsBeatCoinsImage;
UnityEngine::GameObject *SettingsBeatCoinsCanvas;
TMPro::TextMeshProUGUI *SettingsBeatCoinsCount;

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

// Loads the config from disk using our modInfo, then returns it for use
// other config tools such as config-utils don't use this config, so it can be removed if those are in use
Configuration& getConfig() {
    static Configuration config(modInfo);
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load();
    getLogger().info("Completed setup!");
}

MAKE_AUTO_HOOK_MATCH(SettingsBeatCoinsMainMenuHider, &MainMenuViewController::DidActivate, void, MainMenuViewController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
{
    SettingsBeatCoinsMainMenuHider(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    if (HasEnteredSettings)
    {
        SettingsBeatCoinsCount->get_gameObject()->SetActive(false);
        SettingsBeatCoinsImage->get_gameObject()->SetActive(false);
    }
}

void DidActivate(HMUI::ViewController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
{
    if(firstActivation)
    {
        HasEnteredSettings = true;
        UnityEngine::GameObject *container = QuestUI::BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

        SettingsBeatCoinsCanvas = QuestUI::BeatSaberUI::CreateCanvas();
        SettingsBeatCoinsCanvas->get_transform()->set_position({2, 2.65, 4.3f});
        SettingsBeatCoinsImage = QuestUI::BeatSaberUI::CreateImage(SettingsBeatCoinsCanvas->get_transform(), QuestUI::BeatSaberUI::Base64ToSprite(BeatCoinsLogo), {-35, 2}, {8, 8});
        SettingsBeatCoinsCount = QuestUI::BeatSaberUI::CreateText(SettingsBeatCoinsCanvas->get_transform(), std::to_string(getModConfig().BeatCoinsCount.GetValue()));

        UnityEngine::UI::Button *PurchaseMultiplayerButton = QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "Purchase Multiplayer", [&]()
        {
            getLogger().info("Current BeatCoins count is %i", getModConfig().BeatCoinsCount.GetValue());
            if(getModConfig().BeatCoinsCount.GetValue() >= 3000)
            {
                getModConfig().BeatCoinsCount.SetValue(getModConfig().BeatCoinsCount.GetValue() - 3000);
                getModConfig().HasBoughtMultiplayer.SetValue(true);
                SettingsBeatCoinsCount->SetText(std::to_string(getModConfig().BeatCoinsCount.GetValue()));
                getLogger().info("BeatCoins count after purchase is %i", getModConfig().BeatCoinsCount.GetValue());
            }
        });
    }
    SettingsBeatCoinsCount->get_gameObject()->SetActive(true);
    SettingsBeatCoinsImage->get_gameObject()->SetActive(true);
    SettingsBeatCoinsCount->SetText(std::to_string(getModConfig().BeatCoinsCount.GetValue()));
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    QuestUI::Init();
    QuestUI::Register::RegisterMainMenuModSettingsViewController(modInfo, DidActivate);

    il2cpp_functions::Init();

    getModConfig().Init(modInfo);

    getLogger().info("Installing hooks...");

    auto& logger = getLogger();
    Hooks::InstallHooks(logger);

    getLogger().info("Installed all hooks!");
}