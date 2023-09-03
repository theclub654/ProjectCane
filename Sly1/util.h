#pragma once
#include <iostream>
//defines a limit
struct LM
{
	float gMin, gMax;

	constexpr LM(float min = 0, float max = 1):
		gMin(min), gMax(max)
	{}
};

float RadNormalize(float param_1);
float GLimitAbs(float param_1, float param_2);
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
