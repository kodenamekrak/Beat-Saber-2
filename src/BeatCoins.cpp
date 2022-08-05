#include "main.hpp"
#include "logos.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"
#include "GlobalNamespace/GameplaySetupViewController.hpp"
#include "GlobalNamespace/ResultsViewController.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/IReadonlyBeatmapData.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/BeatmapData.hpp"
#include "System/Threading/Tasks/Task_1.hpp"
#include "GlobalNamespace/ScoreModel.hpp"
#include "GlobalNamespace/IBeatmapDataBasicInfo.hpp"

using namespace GlobalNamespace;
int GainedBeatCoins;
bool triggered; 
float percentage, maxScore, Score;

HMUI::ImageView *BeatCoinsImage;
UnityEngine::GameObject *BeatCoinsCanvas;
TMPro::TextMeshProUGUI *BeatCoinsCount;

void calculateMaxScore(int blockCount)
{
    // Wasnt sure how to properly calculate max score due to the first few blocks so took this from ScorePercentage
    if (blockCount < 14)
    {
        if (blockCount == 1)
            maxScore = 115;
        else if (blockCount < 5)
            maxScore = (blockCount - 1) * 230 + 115;
        else
            maxScore = (blockCount - 5) * 460 + 1035;
    }
    else
        maxScore = (blockCount - 13) * 920 + 4715;
    getLogger().info("maxScore for this song is %f", maxScore);
}

MAKE_AUTO_HOOK_MATCH(BeatCoinsDisplayUpdater, &GameplaySetupViewController::DidActivate, void, GameplaySetupViewController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
{
    BeatCoinsDisplayUpdater(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    if (firstActivation)
    {   // create beatcoins image and counter
        BeatCoinsCanvas = QuestUI::BeatSaberUI::CreateCanvas();
        BeatCoinsCanvas->get_transform()->set_position({2, 2.65, 4.3f});
        BeatCoinsImage = QuestUI::BeatSaberUI::CreateImage(BeatCoinsCanvas->get_transform(), QuestUI::BeatSaberUI::Base64ToSprite(BeatCoinsLogo), {-35, 2}, {8, 8});
        BeatCoinsCount = QuestUI::BeatSaberUI::CreateText(BeatCoinsCanvas->get_transform(), std::to_string(getModConfig().BeatCoinsCount.GetValue()));
        triggered = true;
    }
    BeatCoinsCount->SetText(std::to_string(getModConfig().BeatCoinsCount.GetValue()));
    BeatCoinsCount->get_gameObject()->SetActive(true);
    BeatCoinsImage->get_gameObject()->SetActive(true);
}

MAKE_AUTO_HOOK_MATCH(BeatCoinsResultsHider, &ResultsViewController::Init, void, ResultsViewController *self, LevelCompletionResults *levelCompletionResults, IReadonlyBeatmapData *transformedBeatmapData, IDifficultyBeatmap *difficultyBeatmap, bool practice, bool newHighScore)
{
    BeatCoinsResultsHider(self, levelCompletionResults, transformedBeatmapData, difficultyBeatmap, practice, newHighScore);
    if (triggered)
    {
        BeatCoinsCount->get_gameObject()->SetActive(false);
        BeatCoinsImage->get_gameObject()->SetActive(false);
    }
    // get our score at the end of a song
    Score = levelCompletionResults->modifiedScore;
    auto beatmapDataTask = difficultyBeatmap->GetBeatmapDataBasicInfoAsync();
    auto beatmapData = beatmapDataTask->get_Result();

    auto notesCount = beatmapData->get_cuttableNotesCount();
    calculateMaxScore(notesCount);

    getLogger().info("Total Score was %f", Score);
    // gives one beatcoin per 1,000,000 score
    for (GainedBeatCoins = 0; Score >= 1000000; GainedBeatCoins++)
    {
        Score = Score - 1000000;
    }
    getModConfig().BeatCoinsCount.SetValue(getModConfig().BeatCoinsCount.GetValue() + GainedBeatCoins);
    getLogger().info("%i BeatCoins were gained with %f score remaining", GainedBeatCoins, Score);
    GainedBeatCoins = 0;

    percentage = (Score / maxScore) * 100;

    if (percentage >= 95.0) // extra beatcoins for accuracy, will probably require song to be atleast 1:30 at some point
    {
        GainedBeatCoins = 5;
    }
    else if (percentage >= 90.0)
    {
        GainedBeatCoins = 3;
    }
    else if (percentage >= 80.0)
    {
        GainedBeatCoins = 2;
    }
    else
    {
        GainedBeatCoins = 1;
    }
    getModConfig().BeatCoinsCount.SetValue(getModConfig().BeatCoinsCount.GetValue() + GainedBeatCoins);
    getLogger().info("%i BeatCoins were gained with a percentage of %f", GainedBeatCoins, percentage);
}

MAKE_AUTO_HOOK_MATCH(BeatCoinsMainMenuHider, &MainMenuViewController::DidActivate, void, MainMenuViewController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
{
    BeatCoinsMainMenuHider(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    if (triggered)
    {
        BeatCoinsCount->get_gameObject()->SetActive(false);
        BeatCoinsImage->get_gameObject()->SetActive(false);
    }

    UnityEngine::UI::Button *multiplayerButton = self->multiplayerButton;
    UnityEngine::GameObject *gameObject = multiplayerButton->get_gameObject();

    if(!getModConfig().HasBoughtMultiplayer.GetValue())
    {
        gameObject->SetActive(false);
    }
    else
    {
        gameObject->SetActive(true);
    }
}