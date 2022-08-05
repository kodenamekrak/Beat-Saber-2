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
#include "UnityEngine/Application.hpp"
#include "GlobalNamespace/GameSongController.hpp"


using namespace GlobalNamespace;
int GainedBeatCoins;
bool triggered; 
float percentage, maxScore, Score, songLength;

int scorePerBeatcoin = 800000;

HMUI::ImageView *BeatCoinsImage;
UnityEngine::GameObject *BeatCoinsCanvas;
TMPro::TextMeshProUGUI *BeatCoinsCount;
TMPro::TextMeshProUGUI *NextBeatCoinProgress;

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

MAKE_AUTO_HOOK_MATCH(SongLengthGetter, &GameSongController::StartSong, void, GameSongController *self, float songTimeOffset)
{
    SongLengthGetter(self, songTimeOffset);

    songLength = self->get_songLength();
    getLogger().info("Song length is %f", songLength);
}

MAKE_AUTO_HOOK_MATCH(BeatCoinsDisplayUpdater, &GameplaySetupViewController::DidActivate, void, GameplaySetupViewController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
{
    BeatCoinsDisplayUpdater(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    if (firstActivation)
    {   // create beatcoins image and counter
        ///BeatCoinsCanvas = QuestUI::BeatSaberUI::CreateCanvas();
        //BeatCoinsCanvas->get_transform()->set_position({2, 2.65, 4.3f});
        BeatCoinsImage = QuestUI::BeatSaberUI::CreateClickableImage(self->get_transform(), QuestUI::BeatSaberUI::Base64ToSprite(BeatCoinsLogo), {176, 47}, {8, 8}, [&]()
        {
            UnityEngine::Application::OpenURL("https://www.youtube.com/watch?v=dQw4w9WgXcQ");
        });
        BeatCoinsCount = QuestUI::BeatSaberUI::CreateText(self->get_transform(), std::to_string(getModConfig().BeatCoinsCount.GetValue()));
        BeatCoinsCount->get_rectTransform()->set_anchoredPosition({210, 45});
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
    //songLength = levelCompletionResults->endSongTime;
    auto beatmapDataTask = difficultyBeatmap->GetBeatmapDataBasicInfoAsync();
    auto beatmapData = beatmapDataTask->get_Result();

    auto notesCount = beatmapData->get_cuttableNotesCount();
    calculateMaxScore(notesCount);

    percentage = (Score / maxScore) * 100;

    if(songLength >= 90.0f)
    { 
    if(percentage >= 98.0)
    {
        GainedBeatCoins = 3;
    }
    else if (percentage >= 95.0) // extra beatcoins for accuracy, will probably require song to be atleast 1:30 at some point
    {
        GainedBeatCoins = 2;
    }
    else if (percentage >= 90.0)
    {
        GainedBeatCoins = 1;
    }    
    getModConfig().BeatCoinsCount.SetValue(getModConfig().BeatCoinsCount.GetValue() + GainedBeatCoins);
    getLogger().info("%i BeatCoins were gained with a percentage of %f", GainedBeatCoins, percentage);

    // gives one beatcoin per 800,000 score
    Score = levelCompletionResults->modifiedScore + getModConfig().BeatCoinProgress.GetValue();
    for (GainedBeatCoins = 0; Score >= scorePerBeatcoin; GainedBeatCoins++)
    {
        Score = Score - scorePerBeatcoin;
    }
    getModConfig().BeatCoinsCount.SetValue(getModConfig().BeatCoinsCount.GetValue() + GainedBeatCoins);
    getLogger().info("%i BeatCoins were gained with %f score remaining", GainedBeatCoins, Score);
    getLogger().info("Level score was %i, Config score is %i, Combined score is %f", levelCompletionResults->modifiedScore, getModConfig().BeatCoinProgress.GetValue(), Score);
    getModConfig().BeatCoinProgress.SetValue(Score + getModConfig().BeatCoinProgress.GetValue());
    }
}

MAKE_AUTO_HOOK_MATCH(BeatCoinsMainMenuHider, &MainMenuViewController::DidActivate, void, MainMenuViewController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
{
    BeatCoinsMainMenuHider(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    if (triggered)
    {
        BeatCoinsCount->get_gameObject()->SetActive(false);
        BeatCoinsImage->get_gameObject()->SetActive(false);
    }
}