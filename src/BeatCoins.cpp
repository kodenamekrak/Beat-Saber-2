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
float percentage, maxScore, userScore, songLength, combinedScore;

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
    { // create beatcoins image and counter
        BeatCoinsCount = QuestUI::BeatSaberUI::CreateText(self->get_transform(), std::to_string(getModConfig().BeatCoinsCount.GetValue()));
        BeatCoinsImage = QuestUI::BeatSaberUI::CreateClickableImage(self->get_transform(), QuestUI::BeatSaberUI::Base64ToSprite(BeatCoinsLogo), {176, 47}, {9, 9}, [&]()
                                                                    { UnityEngine::Application::OpenURL("https://www.youtube.com/watch?v=dQw4w9WgXcQ"); });
        BeatCoinsCount->get_rectTransform()->set_anchoredPosition({210, 45});
    }
    BeatCoinsCount->SetText(std::to_string(getModConfig().BeatCoinsCount.GetValue()));
}

MAKE_AUTO_HOOK_MATCH(BeatCoinsResultsHider, &ResultsViewController::Init, void, ResultsViewController *self, LevelCompletionResults *levelCompletionResults, IReadonlyBeatmapData *transformedBeatmapData, IDifficultyBeatmap *difficultyBeatmap, bool practice, bool newHighScore)
{
    BeatCoinsResultsHider(self, levelCompletionResults, transformedBeatmapData, difficultyBeatmap, practice, newHighScore);

    // get our score at the end of a song
    userScore = levelCompletionResults->modifiedScore;

    auto beatmapDataTask = difficultyBeatmap->GetBeatmapDataBasicInfoAsync();
    auto beatmapData = beatmapDataTask->get_Result();
    int notesCount = beatmapData->get_cuttableNotesCount();
    calculateMaxScore(notesCount);

    percentage = (userScore / maxScore) * 100;

    if (songLength >= 90.0f)
    {
        if (percentage >= 98.0)
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
        combinedScore = levelCompletionResults->modifiedScore + getModConfig().BeatCoinProgress.GetValue();
        getLogger().info("Level score was %f, Config score is %i, Combined score is %f", userScore, getModConfig().BeatCoinProgress.GetValue(), combinedScore);

        while(combinedScore >= 800000.0f)
        {
            combinedScore -= 800000.0f;
            GainedBeatCoins++;
        }
        
        getModConfig().BeatCoinsCount.SetValue(getModConfig().BeatCoinsCount.GetValue() + GainedBeatCoins);
        getLogger().info("%i BeatCoins were gained with %f score remaining", GainedBeatCoins, combinedScore);
        getModConfig().BeatCoinProgress.SetValue(combinedScore);
    }
}