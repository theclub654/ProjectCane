#pragma once

#include "dec.h"

struct CHKMGR;
struct CHKPNT;

enum DPS 
{
    DPS_Nil = -1,
    DPS_Normal = 0,
    DPS_GivenCharms = 1,
    DPS_Max = 2
};

struct DIFFICULTYRICH
{
    float uSuckRaw;
    int uRichMin;
    int uRichMax;
};

struct DIFFICULTYCONFIG
{
	LM lmSuck;    
	LM lmSuckWorldLoad;
	float duSuckDeathImmediate;      
	float duSuckChkpntNew;           
	float duSuckChkpntRepeat;       
	int uRichLowMin;                 
	int uRichLowMax;                 
	int uRichLowMinHard;             
	int uRichLowMaxHard;             
	float uSuckCharmLow;             
	int ccharmLow;                   
	float uSuckCharmHigh;            
	int ccharmHigh;                  
	float duSuckDeath;               
	float duSuckGameOver;            
	float duSuckRichHigh;
	float duSuckBreak;
	int clifeLow;                    
	float duSuckLowLife;             
	int cRichHighRange;              
	DIFFICULTYRICH* aRichHighRange;  
};

// This sets the difficulty for each level
struct DIFFICULTY
{
    int ccoinRichMin;
    int ccoinRichMax;
    int ccoinPoorMin;
    int ccoinPoorMax;
    int uRichLow;
    int uRichHigh;
    DIFFICULTYCONFIG* pdifficultyconfig;
    DPS dps;
};

DIFFICULTY* PdifficultyEnsureSw();
void OnDifficultyGameLoad(DIFFICULTY* pdifficulty);
void OnDifficultyWorldPreLoad(DIFFICULTY* pdifficulty);
void OnDifficultyWorldPostLoad(DIFFICULTY* pdifficulty);
void OnDifficultyPlayerDeath(DIFFICULTY* pdifficulty, float scale);
void OnDifficultyTriggerCheckpoint(DIFFICULTY* pdifficulty, CHKPNT* pchkpnt);
void OnDifficultyCollectKey(DIFFICULTY* pdifficulty);
void OnDifficultyBreak(DIFFICULTY* pdifficulty, glm::vec3* ppos, int ccoin, int fNoDifficultyUpdate);
void ChangeSuck(DIFFICULTY* pdifficulty, float uSuck);

extern DIFFICULTY g_difficulty;
extern DIFFICULTYCONFIG g_difficultyEasy;
extern DIFFICULTYCONFIG g_difficultyMedium;
extern DIFFICULTYCONFIG g_difficultyHard;
