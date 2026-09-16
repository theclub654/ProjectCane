#pragma once
#include "expl.h"

class EXPLO : public EXPL
{
	public:
	std::shared_ptr <EMITB> pemitb;
	short oidReference;
	short oidShape;
};

EXPLO* NewExplo();
EMITOK* PemitbEnsureExploEmitok(EXPLO* pexplo, ENSK ensk);
glm::vec3* PemitbEnsureExploPosOrigin(EXPLO* pexplo, ENSK ensk);
glm::vec3* PemitbEnsureExploPosMin(EXPLO* pexplo, ENSK ensk);
glm::vec3* PemitbEnsureExploPosMax(EXPLO* pexplo, ENSK ensk);
EMITNK* PemitbEnsureExploEmitnk(EXPLO* pexplo, ENSK ensk);
glm::vec3* PemitbEnsureExploVec(EXPLO* pexplo, ENSK ensk);
LM* PemitbEnsureExploLmSOffset(EXPLO* pexplo, ENSK ensk);
void* PemitbEnsureExploEmitvk(EXPLO* pexplo, ENSK ensk);
LM* PemitbEnsureExploLmSv(EXPLO* pexplo, ENSK ensk);
float* PemitbEnsureExploRSvz(EXPLO* pexplo, ENSK ensk);
LM* PemitbEnsureExploLmTilt(EXPLO* pexplo, ENSK ensk);
int* PemitbEnsureExploCParticlePerRing(EXPLO* pexplo, ENSK ensk);
float* PemitbEnsureExploURandomRad(EXPLO* pexplo, ENSK ensk);
LM* PemitbEnsureExploLmDtSkip(EXPLO* pexplo, ENSK ensk);
void PemitbSetExploEmitok(EXPLO* pexplo, int value);
void PemitbSetExploPosOrigin(EXPLO* pexplo, glm::vec3 value);
void PemitbSetExploPosMin(EXPLO* pexplo, glm::vec3 value);
void PemitbSetExploPosMax(EXPLO* pexplo, glm::vec3 value);
void PemitbSetExploEmitnk(EXPLO* pexplo, int value);
void PemitbSetExploVec(EXPLO* pexplo, glm::vec3 value);
void PemitbSetExploLmSOffset(EXPLO* pexplo, glm::vec2 value);
void PemitbSetExploEmitvk(EXPLO* pexplo, int value);
void PemitbSetExploLmSv(EXPLO* pexplo, glm::vec2 value);
void PemitbSetExploRSvz(EXPLO* pexplo, float value);
void PemitbSetExploLmTilt(EXPLO* pexplo, glm::vec2 value);
void PemitbSetExploCParticlePerRing(EXPLO* pexplo, int value);
void PemitbSetExploURandomRad(EXPLO* pexplo, float value);
void PemitbSetExploLmDtSkip(EXPLO* pexplo, glm::vec2 value);
void* GetExploOidReference(EXPLO* pexplo);
void SetExploOidReference(EXPLO* pexplo, int oidReference);
void* GetExploOidShape(EXPLO* pexplo);
void SetExploOidShape(EXPLO* pexplo, int oidShape);
void InitExplo(EXPLO* pexplo);
int  GetExploSize();
void LoadExploFromBrx(EXPLO* pexplo, CBinaryInputStream* pbis);
void CloneExplo(EXPLO* pexplo, EXPLO* pexploBase);
void BindExplo(EXPLO* pexplo);
std::shared_ptr<EMITB> PemitbCopyOnWrite(const std::shared_ptr <EMITB>& pemitb);
EMITB* PemitbEnsureExplo(EXPLO* pexplo, ENSK ensk);
void AddExploSkeleton(EXPLO* pexplo, OID oid, OID oidOther, float sRadius, float gDensity, float sRadiusOther, float gDensityOther);
void SetExploRipt(EXPLO* pexplo, RIPT ript);
void StandardSmokeCloud(glm::vec3* ppos, float sRadius);
void DeleteExplo(EXPLO* pexplo);