#pragma once

enum DPS {
    DPS_Nil = -1,
    DPS_Normal = 0,
    DPS_GivenCharms = 1,
    DPS_Max = 2
};

// This sets the difficulty for each level
struct DIFFICULTY
{
    int ccoinBrkRichMin;
    int ccoinBrkRichMax;
    int ccoinBrkPoorMin;
    int ccoinBrkPoorMax;
    int ccoinBrkRichLowMax;
    float uBrkRichLow;
    int ccoinBrkRichHighMin;
    float uBrkRichHigh;
    int ccoinNpcMin;
    int ccoinNpcMax;
    int ccoinMiniNpcMin;
    int ccoinMiniNpcMax;
    int clifeMaxExtraCcoin;
    float uSuckCharmLow;
    int ccharmLow;
    float uSuckCharmHigh;
    int ccharmHigh;
    float duSuckDeath;
    float duSuckChkpntTrigger;
    float duSuckChkpntRetrigger;
    DPS dps;
};

// Loading difficulty for world
void OnDifficultyWorldPreLoad(DIFFICULTY *pdifficulty);

static DIFFICULTY g_difficulty;