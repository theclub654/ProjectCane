#include "wr.h"
#include "glob.h"

WR* NewWr()
{
	return new WR{};
}

void InitWr(WR* pwr)
{
	InitLo(pwr);

	pwr->iwreCur = -1;
	pwr->tLastUpdate = -1.0;
	pwr->imatHalf = -1;
}

void HandleWrMessage(WR* pwr, MSGID msgid, void* pv)
{
	if ((msgid < MSGID_Max) && (MSGID_damaged < msgid))
		pwr->iwreCur = (int)pv;
}

void CloneWr(WR* pwr, WR* pwrBase)
{
	pwr->tLastUpdate = pwrBase->tLastUpdate;
	pwr->cwre = pwrBase->cwre;
	pwr->cmat = pwrBase->cmat;
	pwr->imatHalf = pwrBase->imatHalf;
	for (int i = 0; i < 8; i++)
		pwr->awre[i] = pwrBase->awre[i];
	pwr->iwreCur = pwrBase->iwreCur;
	for (int i = 0; i < 4; i++)
		pwr->amatDpos[i] = pwrBase->amatDpos[i];
	for (int i = 0; i < 4; i++)
		pwr->amatDuv[i] = pwrBase->amatDuv[i];
	pwr->pwrbgFirst = pwrBase->pwrbgFirst;
	pwr->fValuesChanged = pwrBase->fValuesChanged;

	CloneLo(pwr, pwrBase);
}

void AddWrCircleWarp(WR* pwr)
{
	const int iwre = pwr->cwre++;
	pwr->iwreCur++;

	WRE* wre = &pwr->awre[iwre];
	wre->wrek = WREK_Circle;

	wre->circle.normal.x = 1.0;
	wre->circle.normal.y = 0.0;
	wre->circle.normal.z = 0.0;

	if (pwr->imatHalf < 0)
	{
		wre->imat = pwr->cmat++;
		pwr->imatHalf = wre->imat;
		wre->circle.is = 0;
	}
	else
	{
		wre->imat = pwr->imatHalf;
		pwr->imatHalf = -1;
		wre->circle.is = 2;
	}
}

WRE* PwreGetWrCircle(WR* pwr, ENSK ensk)
{
	return PwreGetWrCur(pwr, ensk, WREK_Circle);
}

glm::vec4* PwreGetWrCircleNormalVector(WR* pwr, ENSK ensk)
{
	return &PwreGetWrCur(pwr, ensk, WREK_Circle)->circle.normal;
}

float* PwreGetWrCircleFrequency(WR* pwr, ENSK ensk)
{
	return &PwreGetWrCur(pwr, ensk, WREK_Circle)->circle.gFrequency;
}

float* PwreGetWrCirclePhase(WR* pwr, ENSK ensk)
{
	return &PwreGetWrCur(pwr, ensk, WREK_Circle)->circle.uPhase;
}

glm::vec4* PwreGetWrCircleDpos0(WR* pwr, ENSK ensk)
{
	return &PwreGetWrCur(pwr, ensk, WREK_Circle)->circle.awref[0].dpos;
}

glm::vec4* PwreGetWrCircleDpos1(WR* pwr, ENSK ensk)
{
	return &PwreGetWrCur(pwr, ensk, WREK_Circle)->circle.awref[1].dpos;
}

glm::vec4* PwreGetWrCircleDuv0(WR* pwr, ENSK ensk)
{
	return &PwreGetWrCur(pwr, ensk, WREK_Circle)->circle.awref[0].duv;
}

glm::vec4* PwreGetWrCircleDuv1(WR* pwr, ENSK ensk)
{
	return &PwreGetWrCur(pwr, ensk, WREK_Circle)->circle.awref[1].duv;
}

void AddWrBendWarp(WR* pwr)
{
	// Allocate a new WRE slot
	int iwre = pwr->cwre++;
	pwr->iwreCur++;

	WRE* wre = &pwr->awre[iwre];
	wre->wrek = WREK_Bend;

	// Copy global normal Y into the circle normal
	wre->circle.normal.x = 0.0;
	wre->circle.normal.y = 1.0;
	wre->circle.normal.z = 0.0;

	// Bend defaults
	wre->bend.onzRadSwivel.uBias = 0.5f;

	// Initialize the first warp reference delta-position from global normal Z
	wre->circle.awref[0].dpos.x = 0.0;
	wre->circle.awref[0].dpos.y = 0.0;
	wre->circle.awref[0].dpos.z = 1.0;


	// Initialize UV delta Z
	wre->circle.awref[0].duv.z = 0.5f;

	// Consume a material index (matches original side-effect)
	pwr->cmat++;
}

WRE* PwreGetWrBend(WR* pwr, ENSK ensk)
{
	return PwreGetWrCur(pwr, ensk, WREK_Bend);
}

glm::vec4* PwreGetWrBendNormalBend(WR* pwr, ENSK ensk)
{
	return &PwreGetWrCur(pwr, ensk, WREK_Bend)->bend.normalBend;
}

glm::vec2* PwreGetWrBendonzRadBendLM(WR* pwr, ENSK ensk)
{
	return (glm::vec2*)&PwreGetWrCur(pwr, ensk, WREK_Bend)->bend.onzRadBend.lm;
}

float* PwreGetWrBendBenduBias(WR* pwr, ENSK ensk)
{
	return &PwreGetWrCur(pwr, ensk, WREK_Bend)->bend.onzRadBend.uBias;
}

void AddWrBendNoise(WR* pwr, float uAmpl, float gFreq, float gPhase, float uRandom)
{
	WRE* pwre = PwreGetWrBend(pwr, ENSK_Set);
	AddOnzOnze(&pwre->bend.onzRadBend, uAmpl, gFreq, gPhase, uRandom);
}

void AddOnzOnze(ONZ* ponz, float uAmpl, float gFreq, float gPhase, float uRandom)
{
	if (ponz->conze >= 4)
		return;

	ONZE* onze    = &ponz->aonze[ponz->conze++];
	onze->uRandom = uRandom;
	onze->uAmpl   = uAmpl;
	onze->gFreq   = gFreq;
	onze->gPhase  = gPhase;
}

glm::vec4* PwreGetWrBendNormalSwivel(WR* pwr, ENSK ensk)
{
	return &PwreGetWrCur(pwr, ensk, WREK_Bend)->bend.normalSwivel;
}

glm::vec2* PwreGetWrBendonzRadSwivelLM(WR* pwr, ENSK ensk)
{
	return (glm::vec2*)&PwreGetWrCur(pwr, ensk, WREK_Bend)->bend.onzRadSwivel.lm;
}

float* PwreGetWrBenduSwiveluBias(WR* pwr, ENSK ensk)
{
	return &PwreGetWrCur(pwr, ensk, WREK_Bend)->bend.onzRadSwivel.uBias;
}

void AddWrSwivelNoise(WR* pwr, float uAmpl, float gFreq, float gPhase, float uRandom)
{
	WRE* pwre = PwreGetWrBend(pwr, ENSK_Set);
	AddOnzOnze(&pwre->bend.onzRadSwivel, uAmpl, gFreq, gPhase, uRandom);
}

WRE* PwreGetWrCur(WR* pwr, ENSK ensk, WREK wrek)
{
	if (pwr->cwre == 0) {
		return nullptr;
	}

	if (ensk == ENSK_Set) {
		pwr->fValuesChanged = 1;
	}

	return pwr->awre + pwr->iwreCur;
}

void SetWrWavelength(WR* pwr, float sWavelength)
{
	WRE* wre = PwreGetWrCircle(pwr, ENSK_Set);

	float rgDot = 0.0;

	if (sWavelength != 0.0) {
		rgDot = 1.0 / sWavelength;
	}

	wre->circle.rgDot = rgDot;
}

void GetWrWavelength(WR* pwr, float* psWavelength)
{
	WRE* wre = PwreGetWrCircle(pwr, ENSK_Set);

	float wavelength = 0.0f;

	if (wre->circle.rgDot != 0.0f) {
		wavelength = 1.0f / wre->circle.rgDot;
	}

	*psWavelength = wavelength;
}

void GetWrBounds(WR* pwr, glm::vec3* pdpos)
{
	glm::vec3 dposSum(0.0f);

	for (int i = 0; i < pwr->cwre; ++i)
	{
		const WRE& wre = pwr->awre[i];

		if (wre.wrek != 0)
			continue;

		const glm::vec3 a = glm::vec3(wre.circle.awref[0].dpos);
		const glm::vec3 b = glm::vec3(wre.circle.awref[1].dpos);

		dposSum += glm::sqrt(a * a + b * b);
	}

	*pdpos = dposSum;
}

int  GetWrSize()
{
	return sizeof(WR);
}

void ApplyWrGlob(WR* pwr, ALO* palo, GLOB* pglob)
{
	if (!pwr || !palo || !pglob || !pglob->pwrbg)
		return;

	auto wrbg = pglob->pwrbg;
	if (!wrbg) return;

	// already attached?
	//if (wrbg->pwr == pwr) return;

	// already in WR list?
	for (auto it = pwr->pwrbgFirst; it; it = it->pwrbgNextWr)
		if (it == wrbg) return;

	wrbg->pwrbgNextWr.reset();
	wrbg->pwrbgNextWr = pwr->pwrbgFirst;
	pwr->pwrbgFirst = wrbg;

	wrbg->pwr = pwr;
	wrbg->palo = palo;
	wrbg->pglob = pglob;

	// --- NEW: allocate per-GLOB warp state ---
	if (!pglob->pwarpGlob)
		return;

	WRBGLOB_GL& w = *pglob->pwarpGlob;

	w.pwr = pwr;
	w.cmat = pwr->cmat;

	if (w.vertexCount <= 0)
		return;

	const size_t totalVec4 = (size_t)w.vertexCount * (size_t)w.cmat;
	const GLsizeiptr bytes = (GLsizeiptr)(totalVec4 * sizeof(glm::vec4));

	w.state.assign(totalVec4, glm::vec4(0.0f));

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, w.ssboState);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bytes, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

float GFromOnz(ONZ* ponz)
{
	float weighted = 0.0f;
	float amplSum = 0.0f;

	const int count = ponz->conze;
	ONZE* e = ponz->aonze;

	for (int i = 0; i < count; ++i, ++e)
	{
		const float noise = UNoise(e->gFreq, e->gPhase, e->uRandom);
		const float ampl = e->uAmpl;

		amplSum += ampl;
		weighted += ampl * noise;
	}

	// Prevent divide-by-zero if there are no entries or all amplitudes are 0
	const float norm = (amplSum != 0.0f) ? (weighted / amplSum) : 0.0f;

	const float biased = UBias(norm, ponz->uBias);

	const float gMin = ponz->lm.gMin;
	return gMin + biased * (ponz->lm.gMax - gMin);
}

float UBias(float u, float uBias)
{
	return u / ((1.0 / uBias - 2.0) * (1.0 - u) + 1.0);
}

float UNoise(float gFreq, float gPhase, float uRandom)
{
	// Time -> phase
	const float x = (gFreq + gFreq) * g_clock.t + gPhase;  // 2*gFreq*t + phase
	const uint32_t n = (uint32_t)x;

	// Base noise at integer lattice points
	const float n0 = Hash01(n);
	const float n1 = Hash01(n + 1);

	// Randomly scale endpoints depending on parity of n (matches original)
	float a, b;
	if ((n & 1u) == 0u)
	{
		a = uRandom * (n0 - 1.0f) + 1.0f;
		b = n1 * uRandom;
	}
	else
	{
		a = n0 * uRandom;
		b = uRandom * (n1 - 1.0f) + 1.0f;
	}

	// Fractional part and smoothstep (Hermite) interpolation
	const float t = x - (float)n;
	const float s = t * t * (3.0f - (t + t));  // t^2 * (3 - 2t)

	return a + s * (b - a);
}

float Hash01(uint32_t n)
{
	uint32_t x = (n << 13) ^ n;

	return (float)(((x * (x * x * 0x3D73u + 0xC0AE5u) + 0x5208DD0Du) & 0x7FFFFFFFu)) * 4.656613e-10f;
}

void WarpWrTransform(WR* pwr, float sWavelengthMin, const glm::vec3* pposSrc, const glm::mat3* pmatSrc, glm::vec3* pposDst, glm::mat3* pmatDst, glm::vec3* pvDst)
{
	glm::vec3 pos = *pposSrc;
	glm::vec3 vel = glm::vec3(0.0f);

	for (int i = 0; i < pwr->cwre; ++i) 
	{
		WRE* pwre = &pwr->awre[i];

		if (pwre->wrek != WREK_Circle) {
			continue;
		}

		const auto& circle = pwre->circle;

		float wavelength = 0.0f;

		if (circle.rgDot != 0.0f) {
			wavelength = 1.0f / circle.rgDot;
		}

		if (wavelength < sWavelengthMin) {
			continue;
		}

		glm::vec3 normal(circle.normal.x, circle.normal.y, circle.normal.z);
		float dot = glm::dot(*pposSrc, normal);
		float phase = (dot * circle.rgDot + g_clock.t * circle.gFrequency + circle.uPhase) * glm::two_pi<float>();

		float gSin;
		float gCos;
		CalculateSinCos(phase, &gSin, &gCos);

		glm::vec3 dpos0(circle.awref[0].dpos.x, circle.awref[0].dpos.y, circle.awref[0].dpos.z);
		glm::vec3 dpos1(circle.awref[1].dpos.x, circle.awref[1].dpos.y, circle.awref[1].dpos.z);

		pos += dpos0 * gCos + dpos1 * gSin;

		if (pvDst != nullptr) {
			vel += dpos1 * gCos - dpos0 * gSin;
		}
	}

	if (pposDst != nullptr) {
		*pposDst = pos;
	}

	if (pvDst != nullptr) {
		*pvDst = vel;
	}

	if (pmatSrc != nullptr && pmatDst != nullptr) {
		for (int i = 0; i < 3; ++i) {
			glm::vec3 basisPos = *pposSrc + (*pmatSrc)[i];
			glm::vec3 warpedBasisPos;

			WarpWrTransform(pwr, sWavelengthMin, &basisPos, nullptr, &warpedBasisPos, nullptr, nullptr);

			(*pmatDst)[i] = warpedBasisPos - pos;
		}
	}
}

void UpdateWrStateVectors(WR* pwr)
{
	auto pwrbg = pwr ? pwr->pwrbgFirst : nullptr;
	if (!pwrbg || !pwr)
	{
		if (pwr) pwr->fValuesChanged = false;
		return;
	}

	while (pwrbg)
	{
		ALO*  palo  = pwrbg->palo;
		GLOB* pglob = pwrbg->pglob;

		if (!palo || !pglob)
		{
			pwrbg = pwrbg->pwrbgNextWr;
			continue;
		}

		// Must have per-glob warp storage allocated (built at pack time, SSBO sized in ApplyWrGlob)
		if (!pglob->pwarpGlob)
		{
			pwrbg = pwrbg->pwrbgNextWr;
			continue;
		}

		WRBGLOB_GL& w = *pglob->pwarpGlob;

		// WR may change cmat at runtime; keep in sync
		const int cmat = pwr->cmat;
		w.pwr = pwr;
		w.cmat = cmat;

		const int vertexCount = w.vertexCount;
		if (vertexCount <= 0 || w.basePos.empty())
		{
			pwrbg = pwrbg->pwrbgNextWr;
			continue;
		}

		// ------------------------------------------------------------
		// 1) Build dmatEnv for envelope (PS2: dmat = weki.dmat; if pdmat exists: dmat = weki.dmat * pdmat)
		// ------------------------------------------------------------
		const WEKI& weki = pwrbg->weki;

		glm::mat4 dmatEnv(1.0f);
		if (weki.wek != WEK_Nil)
		{
			dmatEnv = weki.dmat;
			if (pglob->pdmat)
				dmatEnv = dmatEnv * (*pglob->pdmat);
		}

		// ------------------------------------------------------------
		// 2) Build CLQ + LM clamp range
		// ------------------------------------------------------------
		float lmMin  = 1.0f, lmMax = 1.0f;
		float clq_g0 = 0.0f, clq_g1 = 0.0f;

		if (weki.wek != WEK_Nil)
		{
			lmMin = (weki.uInner < weki.uOuter) ? weki.uInner : weki.uOuter;
			lmMax = (weki.uInner > weki.uOuter) ? weki.uInner : weki.uOuter;

			const float denom = (weki.sInner - weki.sOuter);
			clq_g1 = (std::abs(denom) > 1e-20f) ? ((weki.uInner - weki.uOuter) / denom) : 0.0f;
			clq_g0 = weki.uInner - clq_g1 * weki.sInner;
		}

		// ------------------------------------------------------------
		// 3) Compute posBendLocal like PS2:
		//    posBendLocal = inverse(weki.dmat) * (axisNormal * midS)
		// ------------------------------------------------------------
		glm::vec3 posBendLocal(0.0f);

		if (weki.wek == WEK_X || weki.wek == WEK_Y || weki.wek == WEK_Z)
		{
			const float midS = 0.5f * (weki.sInner + weki.sOuter);

			glm::vec3 axis(0.0f);
			if (weki.wek == WEK_X) axis = glm::vec3(1.0f, 0.0f, 0.0f);
			if (weki.wek == WEK_Y) axis = glm::vec3(0.0f, 1.0f, 0.0f);
			if (weki.wek == WEK_Z) axis = glm::vec3(0.0f, 0.0f, 1.0f);

			// PS2 uses inverse(weki.dmat) (not the pdmat-composed one)
			const glm::mat4 invWeKi = glm::inverse(weki.dmat);
			posBendLocal = glm::vec3(invWeKi * glm::vec4(axis * midS, 1.0f));
		}

		// ------------------------------------------------------------
		// 4) Build posMat for world position evaluation:
		//    posMat = alo->xf.matWorld (with translation set to alo->xf.posWorld) then * pdmat
		// ------------------------------------------------------------
		glm::mat4 posMat = palo->xf.matWorld;
		posMat[3] = glm::vec4(palo->xf.posWorld, 1.0f);

		if (pglob->pdmat)
			posMat = posMat * (*pglob->pdmat);

		// ------------------------------------------------------------
		// 5) worldPosBend = alo.posWorld + (alo.rot * posBendLocal)
		// ------------------------------------------------------------
		const glm::vec3 worldPosBend = palo->xf.posWorld + (glm::mat3(palo->xf.matWorld) * posBendLocal);

		// ------------------------------------------------------------
		// 6) Ensure state sized & cleared
		// ------------------------------------------------------------
		const size_t totalVec4 = (size_t)cmat * (size_t)vertexCount;

		if (w.state.size() != totalVec4)
			w.state.assign(totalVec4, glm::vec4(0.0f));
		else
			std::fill(w.state.begin(), w.state.end(), glm::vec4(0.0f));

		// ------------------------------------------------------------
		// 7) Compute au[v] using dmatEnv (envelope only)
		// ------------------------------------------------------------
		std::vector<float> au((size_t)vertexCount, 1.0f);

		if (weki.wek != WEK_Nil)
		{
			for (int v = 0; v < vertexCount; ++v)
			{
				const glm::vec3 pEnv = glm::vec3(dmatEnv * w.basePos[v]);

				float s = 0.0f;
				switch (weki.wek)
				{
					case WEK_X:   s = pEnv.x; break;
					case WEK_Y:   s = pEnv.y; break;
					case WEK_Z:   s = pEnv.z; break;
					case WEK_XY:  s = std::sqrt(pEnv.x * pEnv.x + pEnv.y * pEnv.y); break;
					case WEK_XZ:  s = std::sqrt(pEnv.x * pEnv.x + pEnv.z * pEnv.z); break;
					case WEK_YZ:  s = std::sqrt(pEnv.y * pEnv.y + pEnv.z * pEnv.z); break;
					case WEK_XYZ: s = std::sqrt(pEnv.x * pEnv.x + pEnv.y * pEnv.y + pEnv.z * pEnv.z); break;
					default:
					au[v] = 1.0f;
					continue;
				}

				float u = clq_g0 + s * clq_g1;
				if (u < lmMin) u = lmMin;
				if (u > lmMax) u = lmMax;
				au[v] = u;
			}
		}

		// --------------------------------------------------------
		// 8) Write per-WRE states into w.state
		//    Layout: state[imat * vertexCount + v]
		// --------------------------------------------------------
		for (int iwre = 0; iwre < pwr->cwre; ++iwre)
		{
			const WRE& wre = pwr->awre[iwre];
			const int imat = wre.imat;

			if (imat < 0 || imat >= cmat)
				continue;

			glm::vec4* dst = w.state.data() + (size_t)imat * (size_t)vertexCount;

			if (wre.wrek == WREK_Circle)
			{
				const int is = wre.circle.is;
				if (is < 0 || is + 1 >= 4)
					continue;

				const glm::vec3 normal = wre.circle.normal;
				const float rgDot = wre.circle.rgDot;
				const float twoPi = 6.283185307179586f;

				for (int v = 0; v < vertexCount; ++v)
				{
					const glm::vec3 Pw = glm::vec3(posMat * w.basePos[v]);

					const float dotv = Pw.x * normal.x + Pw.y * normal.y + Pw.z * normal.z;
					const float ang = dotv * rgDot * twoPi;

					const float sn = std::sinf(ang);
					const float cs = std::cosf(ang);

					const float u = au[v];

					// Circle writes only lanes is and is+1
					dst[v][is] = cs * u;
					dst[v][is + 1] = sn * u;
				}
			}
			else if (wre.wrek == WREK_Bend)
			{
				for (int v = 0; v < vertexCount; ++v)
				{
					const glm::vec3 Pw = glm::vec3(posMat * w.basePos[v]);
					const glm::vec3 d = Pw - worldPosBend;

					const float u = au[v];

					// Bend writes xyz = u*d, w=1
					dst[v].x = d.x * u;
					dst[v].y = d.y * u;
					dst[v].z = d.z * u;
					dst[v].w = 1.0f;
				}
			}
		}

		// --------------------------------------------------------
		// 9) Upload state SSBO ONCE for this glob
		// --------------------------------------------------------
		if (w.ssboState != 0)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, w.ssboState);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, (GLsizeiptr)(w.state.size() * sizeof(glm::vec4)), w.state.data());
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}

		pwrbg = pwrbg->pwrbgNextWr;
	}

	pwr->fValuesChanged = false;
}

void UpdateWrMatrixes(WR* pwr)
{
	if (g_clock.t == pwr->tLastUpdate)
		return;

	if (pwr->fValuesChanged == true)
		UpdateWrStateVectors(pwr);

	for (int i = 0; i < pwr->cwre; ++i)
	{
		WRE& wre = pwr->awre[i];
		const int imat = wre.imat;

		if (imat < 0 || imat >= pwr->cmat)
			continue;

		glm::mat4& mDpos = pwr->amatDpos[imat];
		glm::mat4& mDuv = pwr->amatDuv[imat];

		if (wre.wrek == WREK_Circle)
		{
			const float angle = (g_clock.t * wre.circle.gFrequency + wre.circle.uPhase) * 6.283185307179586f;
			const float s = std::sinf(angle);
			const float c = std::cosf(angle);

			const int is = wre.circle.is;
			if (is < 0 || is + 1 >= 4)
				continue;

			// Clear only the touched columns to avoid stale values in z/w etc.
			mDpos[is]     = glm::vec4(0.0f);
			mDpos[is + 1] = glm::vec4(0.0f);
			mDuv[is]      = glm::vec4(0.0f);
			mDuv[is + 1]  = glm::vec4(0.0f);

			const glm::vec3 ref0Pos(wre.circle.awref[0].dpos.x, wre.circle.awref[0].dpos.y, wre.circle.awref[0].dpos.z);
			const glm::vec3 ref1Pos(wre.circle.awref[1].dpos.x, wre.circle.awref[1].dpos.y, wre.circle.awref[1].dpos.z);

			// PS2:
			// col(is)   = ref0*c + ref1*s
			// col(is+1) = ref1*c + ref0*(-s) = ref1*c - ref0*s
			const glm::vec3 v0 = ref0Pos * c + ref1Pos * s;
			const glm::vec3 v1 = ref1Pos * c - ref0Pos * s;

			// Write columns is and is+1 (rows 0..2 are xyz)
			mDpos[is][0] = v0.x;  mDpos[is][1] = v0.y;  mDpos[is][2] = v0.z;
			mDpos[is + 1][0] = v1.x;  mDpos[is + 1][1] = v1.y;  mDpos[is + 1][2] = v1.z;

			const glm::vec2 ref0Uv(wre.circle.awref[0].duv.x, wre.circle.awref[0].duv.y);
			const glm::vec2 ref1Uv(wre.circle.awref[1].duv.x, wre.circle.awref[1].duv.y);

			const glm::vec2 uv0 = ref0Uv * c + ref1Uv * s;
			const glm::vec2 uv1 = ref1Uv * c - ref0Uv * s;

			// UV goes into rows 0..1 (x,y). z/w stay 0.
			mDuv[is][0] = uv0.x;  mDuv[is][1] = uv0.y;
			mDuv[is + 1][0] = uv1.x;  mDuv[is + 1][1] = uv1.y;
		}
		else if (wre.wrek == WREK_Bend)
		{
			// PS2 writes a delta (R - I) into the 3x3 upper-left portion.
			// Clear the whole matrix so untouched lanes don't keep old junk.
			mDpos = glm::mat4(0.0f);

			const float radBend   = GFromOnz(&wre.bend.onzRadBend);
			const float radSwivel = GFromOnz(&wre.bend.onzRadSwivel);

			glm::vec3 normalSwivel(wre.bend.normalSwivel.x, wre.bend.normalSwivel.y, wre.bend.normalSwivel.z);
			glm::vec3 normalIn(wre.bend.normalBend.x, wre.bend.normalBend.y, wre.bend.normalBend.z);

			// Swivel rotate
			const float ns2 = glm::dot(normalSwivel, normalSwivel);
			if (ns2 > 1e-20f) normalSwivel *= glm::inversesqrt(ns2);
			else normalSwivel = glm::vec3(0, 0, 1);

			const glm::mat3 matSwivel = glm::mat3(glm::rotate(glm::mat4(1.0f), radSwivel, normalSwivel));

			// Apply swivel to bend axis, normalize
			glm::vec3 axis = matSwivel * normalIn;
			const float len2 = glm::dot(axis, axis);
			if (len2 > 1e-20f) axis *= glm::inversesqrt(len2);
			else axis = glm::vec3(0, 0, 1);

			// Bend rotate
			const glm::mat3 bendR = glm::mat3(glm::rotate(glm::mat4(1.0f), radBend, axis));

			// Store (R - I) in upper-left 3x3.
			// GLM: m[col][row]
			mDpos[0][0] = bendR[0][0] - 1.0f;  mDpos[0][1] = bendR[0][1];        mDpos[0][2] = bendR[0][2];
			mDpos[1][0] = bendR[1][0];         mDpos[1][1] = bendR[1][1] - 1.0f; mDpos[1][2] = bendR[1][2];
			mDpos[2][0] = bendR[2][0];         mDpos[2][1] = bendR[2][1];        mDpos[2][2] = bendR[2][2] - 1.0f;
		}
	}

	pwr->tLastUpdate = g_clock.t;
}

void DeleteWr(WR* pwr)
{
	delete pwr;
}