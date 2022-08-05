#pragma once

#include "config-utils/shared/config-utils.hpp"


DECLARE_CONFIG(ModConfig,
    CONFIG_VALUE(BeatCoinsCount, int, "Current BeatCoins", 0);
    CONFIG_VALUE(BeatCoinProgress, int, "Progress to next beatcoin", 0);

    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(BeatCoinsCount);
        CONFIG_INIT_VALUE(BeatCoinProgress);


    )
)