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

float GSmoothA(float gCur, float dgCur, float gTarget, float dt, SMPA* psmpa, float* pdgNext)
{
	float dgNext;
	float gNext = GSmooth(gCur, gTarget, dt, reinterpret_cast<SMP*>(psmpa), &dgNext);

	const float sdvMax = psmpa->sdvMax;

	if (sdvMax > 0.0f)
	{
		const float dgMax = dgCur + sdvMax * dt;
		const float dgMin = dgCur - sdvMax * dt;
		const float dgStep = 0.5f * sdvMax * dt * dt;

		if (dgNext > dgMax)
		{
			gNext = gCur + dgCur * dt + dgStep;
			dgNext = dgMax;
		}
		else if (dgNext < dgMin)
		{
			gNext = gCur + dgCur * dt - dgStep;
			dgNext = dgMin;
		}
	}

	if (pdgNext != nullptr)
		*pdgNext = dgNext;

	return gNext;
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
			velocity = -vMid;
			delta = (vSlow * t) + (0.5f * accelRate * t * t);
		}
		else {
			velocity = -vSlow;
			delta = 0.0f;
		}
	}

	float result = isNegative ? gTarget - delta : gTarget + delta;
	if (pdgNext != nullptr) {
		*pdgNext = isNegative ? -velocity : velocity;
	}
	return result;
}

float RadSmooth(float radCur, float radTarget, float dt, SMP* psmp, float* pdradNext)
{
	float drad = RadNormalize(radTarget - radCur);
	float dradSmoothed = GSmooth(0.0f, drad, dt, psmp, pdradNext);

	return RadNormalize(radCur + dradSmoothed);
}

float RadSmoothA(float radCur, float dradCur, float radTarget, float dt, SMPA* psmpa, float* pdradNext)
{
	const float dradTarget = RadNormalize(radTarget - radCur);
	const float drad = GSmoothA(0.0f, dradCur, dradTarget, dt, psmpa, pdradNext);

	return RadNormalize(radCur + drad);
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
	float t;

	if (gLow == gHigh) {
		return gLow;
	}

	t = (float)rand() / ((float)RAND_MAX + 1.0f);

	return gLow + (gHigh - gLow) * t;
}

//return a random number from the Gaussian distribution
float GRandGaussian(float gMean, float gDeviation, float gLimit)
{
	float x;
	float y;
	float radiusSq;
	float gaussian;
	float result;

	do {
		do {
			x = GRandInRange(-1.0f, 1.0f);
			y = GRandInRange(-1.0f, 1.0f);

			radiusSq = x * x + y * y;
		} while (radiusSq > 1.0f);
	} while (radiusSq == 0.0f);

	gaussian = y * sqrt((-2.0f * logf(radiusSq)) / radiusSq);

	result = gMean + gDeviation * gaussian;

	if (gLimit != 0.0f) {
		if (result < gMean - gLimit) {
			result = gMean - gLimit;
		}
		else if (result > gMean + gLimit) {
			result = gMean + gLimit;
		}
	}

	return result;
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

void PrescaleClq(CLQ* src, float ru, float du, CLQ* dst)
{
	if (src == nullptr || dst == nullptr)
		return;

	dst->g2 = src->g2 * ru * ru;
	dst->g1 = (2.0f * src->g2 * ru * du) + (src->g1 * ru);
	dst->g0 = (src->g2 * du * du) + (src->g1 * du) + src->g0;
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
	return g > plm->gMin && g < plm->gMax;
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
	if (g < plm->gMin)
		return plm->gMin;

	if (g > plm->gMax)
		return plm->gMax;

	return g;
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

void MinimizeRange(PFNGG pfn, void* pv, float g, float dg, float gMin, float gMax, float* pgDom, float* pgRng)
{
	auto Evaluate = [pfn, pv](float gDom) -> DR
	{
		return { gDom, pfn(pv, gDom) };
	};

	DR left;
	DR center;
	DR right;

	center = Evaluate(std::min(std::max(g, gMin), gMax));
	left = Evaluate(std::max(center.gDom - dg, gMin));
	right = Evaluate(std::min(center.gDom + dg, gMax));

	while (center.gRng > left.gRng)
	{
		right = center;
		center = left;
		left = Evaluate(std::max((2.0f * center.gDom) - right.gDom, gMin));
	}

	while (center.gRng > right.gRng)
	{
		left = center;
		center = right;
		right = Evaluate(std::min((2.0f * center.gDom) - left.gDom, gMax));
	}

	for (int iteration = 0; iteration < 10; ++iteration)
	{
		const DR leftMid = Evaluate((left.gDom + center.gDom) * 0.5f);
		const DR rightMid = Evaluate((center.gDom + right.gDom) * 0.5f);

		if (leftMid.gRng < center.gRng)
		{
			if (leftMid.gRng < rightMid.gRng)
			{
				right = center;
				center = leftMid;
			}
			else
			{
				left = center;
				center = rightMid;
			}
		}
		else if (center.gRng <= rightMid.gRng)
		{
			left = leftMid;
			right = rightMid;
		}
		else
		{
			left = center;
			center = rightMid;
		}
	}

	if (pgDom != nullptr)
		*pgDom = center.gDom;

	if (pgRng != nullptr)
		*pgRng = center.gRng;
}

//TODO: MinimizeRange

LM g_lmZeroOne = { 0.0, 1.0 };
CLQ g_clqZero{0.0};
