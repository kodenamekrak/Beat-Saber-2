#include <iostream>

int test;

int main()
{
    srd::cin >> test;
    calculateMaxScore(input);
}

void calculateMaxScore(int blockCount){
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
}