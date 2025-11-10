#include "util.h"
#include <math.h>

static constexpr float PI = 3.141593f;

float RadNormalize(float angle)
{
	if ((angle < -PI) || (PI < angle))
	{
		float gMod = GModPositive(angle + PI, PI*2);
		return gMod - PI;
	}
	return angle;
}

//clamp the float to fall within the interval [-absLimit, absLimit]
float GLimitAbs(float g, float absLimit)
{
	if (g <= absLimit)
	{
		if (g < -absLimit)
		{
			g = -absLimit;
		}
		return g;
	}
	return absLimit;
}

float GSmooth(float gCur, float gTarget, float dt, SMP* psmp, float* pdgNext)
{
	float delta = gCur - gTarget;
	bool isNegative = delta < 0.0f;
	if (isNegative) delta = -delta;

	float acc = psmp->svFast;
	float vSlow = psmp->svSlow;
	float dtFast = psmp->dtFast;

	float accelDist = (vSlow + acc) * dtFast * 0.5f;
	float velocity = 0.0f;
	bool finalStepHandled = false;

	if (accelDist <= delta) {
		float accelTime = (delta - accelDist) / acc;
		if (dt <= accelTime) {
			velocity = -acc;
			delta -= acc * dt;
			finalStepHandled = true;
		}
		else {
			dt -= accelTime;
			delta = accelDist;
		}
	}

	if (!finalStepHandled) {
		float tSolutions[2] = { 0.0f, 0.0f };
		float a = (acc - vSlow) / (2.0f * dtFast);
		float b = vSlow;
		float c = -delta;

		if (CSolveQuadratic(a, b, c, &tSolutions[0]) && tSolutions[0] > dt) {
			float t = tSolutions[0] - dt;
			float accelRate = (acc - vSlow) / dtFast;
			float vMid = vSlow + accelRate * t;
			velocity = -0.5f * (vSlow + vMid);
			delta = (vSlow * t) + (0.5f * accelRate * t * t);
		}
		else {
			velocity = -vSlow;
			delta = vSlow * dt;
		}
	}

	float result = isNegative ? gTarget - delta : gTarget + delta;
	if (pdgNext != nullptr) {
		*pdgNext = isNegative ? -velocity : velocity;
	}
	return result;
}

//TODO: GSmooth
//TODO: GSmoothA
//TODO: RadSmooth
//TODO: RadSmoothA
//TODO: PosSmooth
//TODO: SmoothMatrix

//returns a random integer in the given range
int NRandInRange(int nLow, int nHigh)
{
	if (nLow != nHigh)
	{
		int nRand = rand() % 0x95675;
		return nLow + nRand % ((nHigh - nLow) + 1);
	}
	return nLow;
}

//return a random floating-point number in the given range
float GRandInRange(float gLow, float gHigh)
{
	if (gLow != gHigh)
	{
		int nRand = rand();
		return gLow + (gHigh - gLow) * (float)nRand * (float)0x30000000;
	}
	return gLow;
}

//return a random number from the Gaussian distribution
float GRandGaussian(float param_1, float param_2, float param_3)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	fVar3 = -1.0f;
	fVar4 = 0.0f;
	do {
		do {
			fVar1 = (float)GRandInRange(fVar3, 1.0f);
			fVar2 = (float)GRandInRange(fVar3, 1.0f);
			fVar1 = fVar1 * fVar1 + fVar2 * fVar2;
		} while (1.0f < fVar1);
	} while (fVar1 == fVar4);
	fVar3 = logf(fVar1);
	fVar4 = param_1 + param_2 * fVar2 * sqrtf((fVar3 * -2.0f) / fVar1);
	fVar3 = fVar4;
	if (param_3 != 0.0f)
	{
		fVar3 = param_1 - param_3;
		if ((param_1 - param_3 <= fVar4) && (fVar3 = fVar4, param_1 + param_3 < fVar4))
			fVar3 = param_1 + param_3;
	}
	return fVar3;
}

//returns true if two given floats they are within a certain epsilon of each other
bool FFloatsNear(float g1, float g2, float gEpsilon)
{
	float fVar1;

	fVar1 = fabs(g1);
	
	return (uint32_t)(fabs(g1 - g2) / (float)((uint32_t)(fVar1 < 1.0) * 1 | (int)fVar1 * (uint32_t)(fVar1 >= 1.0)) < gEpsilon);
}

//solve a quadratic equation of the form ax^2+bx+c
int CSolveQuadratic(float a, float b, float c, float *solutions)
{
	float delta = b * b - a * c * 4.0;
	if (delta < 0.0)
		return 0; //no solution

	float sqrtOver2a = sqrtf(delta) / (a + a);
	float bOver2a = b / (a + a);
	if (fabs(sqrtOver2a) < 0.0001f)
	{
		//one solution
		solutions[0] = -bOver2a;
		return 1;
	}
	
	//two solutions
	solutions[0] = -bOver2a + sqrtOver2a;
	solutions[1] = -bOver2a - sqrtOver2a;
	return 2;
}

//TODO: PrescaleClq

double GTrunc(double param_1)
{
	uint64_t uVar1;
	uint64_t uVar2;
	int iVar3;

	iVar3 = ((uint32_t)((uint64_t)param_1 >> 0x34) & 0x7ff) - 0x3ff;
	if (iVar3 < 0)
		param_1 = 0.0;
	else
	{
		if (iVar3 < 0x34)
		{
			uVar2 = (uint64_t)param_1 & 0xfffffffffffff;
			uVar1 = (1 << (long)(0x34 - iVar3)) - 1;
			if ((uVar2 & uVar1) == uVar1)
				param_1 = (double)((uint64_t)param_1 & 0xfff0000000000000 | uVar2 & ~uVar1) + 1.0;
			else
				param_1 = (double)((uint64_t)param_1 & 0xfff0000000000000 | uVar2 & ~uVar1);
		}
	}
	return param_1;
}

float GTrunc(float param_1)
{
	uint32_t uVar1;
	int iVar2;
	uint32_t uVar3;
	uint64_t local_10;

	local_10 = (uint64_t)(uint32_t)param_1;
	iVar2 = ((uint32_t)((local_10 << 9) >> 0x20) & 0xff) - 0x7f;
	if (iVar2 < 0)
		param_1 = 0.0;
	else
	{
		if (iVar2 < 0x17)
		{
			uVar1 = (int)(1 << (long)(0x17 - iVar2)) - 1;
			uVar3 = (uint32_t)param_1 & 0x7fffff;
			if ((uVar3 & uVar1) == uVar1)
				param_1 = (float)((uint32_t)param_1 & 0xff800000 | uVar3 & ~uVar1) + 1.0f;
			else
				param_1 = (float)((uint32_t)param_1 & 0xff800000 | uVar3 & ~uVar1);
		}
	}
	return param_1;
}

float GModPositive(float gDividend, float gDivisor)
{
	float result = fmodf(gDividend, gDivisor);
	if (result < 0.0f)
		result += gDivisor;
	return result;
}

//TODO: FitClq

//check whether the given float falls within the given limit
bool FCheckLm(LM *plm, float g)
{
	return (plm->gMin < g) && (g < plm->gMax);
}

//check whether the given float falls within any of the given limits
bool FCheckAlm(int clm, LM *alm, float g)
{
	int lmCur = 0;
	if (0 < clm)
	{
		do
		{
			if (FCheckLm(alm, g))
				return true;
			lmCur++;
			alm++;
		} while (lmCur < clm);
	}
	return false;
}

//clamp the float to fall inside range given by the limit
float GLimitLm(LM *plm, float g)
{
	float result = plm->gMin;

	if ((result <= g) && (result = plm->gMax, g <= plm->gMax))
		return g;

	return plm->gMin;
}

int SgnCompareG(float *a, float *b)
{	
	if (*a > *b)
		return 1;
	else if (*a < *b)
		return -1;
	else
		return 0;
}

void Force(void *)
{
	//this function is empty
}

//TODO: MinimizeRange

LM g_lmZeroOne = { 0.0, 1.0 };