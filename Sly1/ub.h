#pragma once
#include "gomer.h"

class UBG : public GOMER
{
	public:
};

class UBP : public SO
{
	public:
};

void InitUbg(UBG* pubg);
void RenderUbpAll(UBP* pubp, CM* pcm, RO* pro);