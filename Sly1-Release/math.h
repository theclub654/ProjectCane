#pragma once
#include "glm/vec3.hpp"
#include "glm/mat3x3.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include <glm/gtx/compatibility.hpp>
#include "util.h"

void LoadMatrixFromPosRot(glm::vec3* ppos, glm::mat3* pmat, glm::mat4* pmatDst);
void LoadMatrixFromPosRotScale(glm::vec3* vecPos, glm::mat3* matRot, glm::vec3* vecScale, glm::mat4* pmat);
void LoadMatrixFromPosRotInverse(glm::vec3* pposSrc, glm::mat3* pmatSrc, glm::mat4* pmatDst);
void LoadRotateMatrixEuler(glm::vec3* peul, glm::mat3* pmatRot);
void LoadRotateMatrix(glm::vec3* pvec, glm::mat3* pmat);
void LoadLookAtMatrix(glm::vec3* pdpos, glm::mat3* pmat);
void DecomposeRotateMatrixRad(glm::mat3* pmat, float* prad, glm::vec3* pnormal);
void TiltMatUpright(const glm::mat3* pmat, glm::mat3* pdmat, glm::mat3* pmatUpright);
void BuildSimpleProjectionMatrix(float rx, float ry, float dxOffset, float dyOffset, float sNear, float sFar, glm::mat4& outMat);
void BuildOrthonormalMatrixZ(glm::vec3* pvecX, glm::vec3* pvecZ, glm::mat3* pmat);
inline void BuildOrthonormalMatrixZ(glm::vec3& vecX, glm::vec3& vecZ, glm::mat3& mat)
{
	BuildOrthonormalMatrixZ(&vecX, &vecZ, &mat);
}
void CalculateDmat4(const glm::mat4& mat0, const glm::mat4& mat1, glm::mat4& dmat);
void DecomposeRotateMatrixEuler(const glm::mat4& mat, glm::vec3& eul);
void LoadScaleMatrixScalar(glm::vec3* ppos, float rScale, glm::mat4* outMatScale);
void LoadScaleMatrixVector(glm::vec3* ppos, glm::mat3* pmat, glm::vec3* pvecScale, glm::mat4* outMatScale);
void BuildRotateVectorsMatrix(const glm::vec3* pvec1, const glm::vec3* pvec2, glm::mat3* pmat);
void LoadRotateMatrixRad(float rad, const glm::vec3* pnormal, glm::mat3* pmat);
void LoadRotateMatrixPanTilt(float radPan, float radTilt, glm::mat3* pmat);
void DecomposeSphere(const glm::vec3* pposEye, const glm::vec3* pposCenter, float* pradPan, float* pradTilt, float* ps);
void CalculateSinCos(float rad, float* pgSin, float* pgCos);
void CalculateDmat(const glm::mat3* pmat0, const glm::mat3* pmat1, glm::mat3* pdmat);
void CalculateVectorPanTilt(const glm::vec3& vec, float* pradPan, float* pradTilt);
void GetNormalVectors(glm::vec3* pvec, glm::vec3* avec, glm::vec3* pnormalTry1, glm::vec3* pnormalTry2);
inline void GetNormalVectors(glm::vec3& vec, glm::vec3& normal1, glm::vec3& normal2,
	glm::vec3& normalTry1, glm::vec3& normalTry2)
{
	glm::vec3 normals[2]{};
	GetNormalVectors(&vec, normals, &normalTry1, &normalTry2);
	normal1 = normals[0];
	normal2 = normals[1];
}
void GetNormalVector(const glm::vec3& vecSrc, glm::vec3& vecDst);
void SetVectorCylind(glm::vec3* pvec, float rad, float sXY, float sZ);
void SetVectorSphere(glm::vec3* pvec, float radPan, float radTilt, float s);
float RadBetweenVectors(glm::vec3* pvecStart, glm::vec3* pvecFinish, glm::vec3* pnormal);
inline float RadBetweenVectors(const glm::vec3* pvecStart, const glm::vec3* pvecFinish, glm::vec3* pnormal)
{
	return RadBetweenVectors(const_cast<glm::vec3*>(pvecStart), const_cast<glm::vec3*>(pvecFinish), pnormal);
}
inline float RadBetweenVectors(const glm::vec3& vecStart, const glm::vec3& vecFinish, glm::vec3* pnormal)
{
	return RadBetweenVectors(&vecStart, &vecFinish, pnormal);
}
void ConvertDeulToW(glm::vec3* peul, glm::vec3* pdeul, glm::vec3* pw);
void FitClq(float g0, float g1, float u, float gU, CLQ* pclq);
void NormalizeRotateMatrix3(glm::mat3* pmat);
float SProjectVector(glm::vec3* pvec, glm::vec3* pnormal, glm::vec3* pvecNormal, glm::vec3* pvecTangent);
void FindClosestPointBetweenLines(glm::vec3* pvecA, glm::vec3* pvecB, glm::vec3* pvecC, glm::vec3* pvecD, float* au, float* ps, glm::vec3* pnormal);
void LimitVectorLength(glm::vec3* pvecSrc, float sMax, glm::vec3* pvecDst);
void DecomposeRotateMatrixPanTilt(const glm::mat3* pmat, float* pradPan, float* pradTilt);
void CalcHeadingVector(float rad, glm::vec3* pvec);
void FindClosestPointBetweenLineSegments(glm::vec3* pvecA, glm::vec3* pvecB, glm::vec3* pvecC, glm::vec3* pvecD, float* au, float* ps, glm::vec3* pnormal);
glm::vec3 PosSmooth(glm::vec3 posCur, glm::vec3 posTarget, float dt, SMP* psmp, glm::vec3* pv);
void SmoothMatrix(const glm::mat3* pmatPrev, const glm::mat3* pmatNext, SMP* psmp, float dt, glm::mat3* pmatSmooth, glm::vec3* pwSmooth);
float CosRotateMatrixMagnitude(glm::mat3* pmat);
void DecomposeCylind(glm::vec3* pposEye, glm::vec3* pposCenter, float* prad, float* psXY, float* psZ);
inline void DecomposeCylind(const glm::vec3* pposEye, const glm::vec3* pposCenter, float* prad, float* psXY, float* psZ)
{
	DecomposeCylind(const_cast<glm::vec3*>(pposEye), const_cast<glm::vec3*>(pposCenter), prad, psXY, psZ);
}
void LoadRotateVectorMatrix(const glm::vec3* pvecFrom, const glm::vec3* pvecTo, glm::mat3* pmat);
void BuildOrthonormalMatrixY(const glm::vec3* pvecX, const glm::vec3* pvecY, glm::mat3* pmat);


extern glm::vec3 g_vecOne;
extern glm::vec3 g_normalX;
extern glm::vec3 g_normalY;
extern glm::vec3 g_normalZ;
extern glm::vec3 g_vecZero;
extern glm::mat3 g_matIdentity;
