#pragma once
#include <iostream>
//defines a limit
struct LM
{
	float gMin;
	float gMax;

	constexpr LM(float min = 0, float max = 1):
		gMin(min), gMax(max)
	{}
};

struct CLQ
{
	float g0;
	float g1;
	float g2;
	float gUnused;
};

struct CLQC 
{
	float g0;
	float g1;
	float g2;
	float g3;
};

struct SMP
{
	float svFast;
	float svSlow;
	float dtFast;
};

struct SMPA 
{
	float svFast;
	float svSlow;
	float dtFast;
	float sdvMax;
};


float RadNormalize(float param_1);
float GLimitAbs(float param_1, float param_2);
float GSmooth(float gCur, float gTarget, float dt, SMP* psmp, float* pdgNext);
//TODO: GSmooth
//TODO: GSmoothA
//TODO: RadSmooth
//TODO: RadSmoothA
//TODO: PosSmooth
//TODO: SmoothMatrix
int NRandInRange(int param_1, int param_2);
float GRandInRange(float param_1, float param_2);
float GRandGaussian(float param_1, float param_2, float param_3);
bool FFloatsNear(float param_1, float param_2, float param_3);
int CSolveQuadratic(float a, float b, float c, float *solutions);
//TODO: PrescaleClq
void CalculateSinCos(float angle, float *sin, float *cos);
double GTrunc(double param_1);
float GTrunc(float param_1);
float GModPositive(float param_1, float param_2);
//TODO: FitClq
bool FCheckLm(LM *plm, float g);
bool FCheckAlm(int clm, LM *alm, float g);
float GLimitLm(LM *plm, float g);
int SgnCompareG(float *a, float *b);
void Force(void *);
//TODO: MinimizeRange
