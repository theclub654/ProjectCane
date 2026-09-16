#pragma once
#include "coin.h"

class BOOST : public DPRIZE
{
	public:

};

BOOST*NewBoost();
void CloneBoost(BOOST* pboost, BOOST* pboostBase);
void InitBoost(BOOST* pboost);
void UpdateBoost(BOOST* pboost, float dt);

static glm::vec3 s_dposBoostCollect = {0.0, 0.0, 200.0};
static float s_frqBoostCollect = -1.0;
static float s_sBoostCollectAngularVelocity = 0.75;