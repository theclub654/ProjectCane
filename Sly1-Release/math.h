#pragma once
#include "glm/vec3.hpp"
#include "glm/mat3x3.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include <glm/gtx/compatibility.hpp>

void LoadMatrixFromPosRot(glm::vec3& ppos, glm::mat3& pmat, glm::mat4& pmatDst);
void LoadMatrixFromPosRotScale(glm::vec3& vecPos, glm::mat3& matRot, glm::vec3& vecScale, glm::mat4& pmat);
void LoadMatrixFromPosRotInverse(glm::vec3& pposSrc, glm::mat3& pmatSrc, glm::mat4& pmatDst);
void BuildOrthonormalMatrixZ(glm::vec3& pvecX, glm::vec3& pvecZ, glm::mat4& pmat);
glm::vec3 DecomposeRotateMatrixEuler(const glm::mat3& R);
void LoadScaleMatrixScalar(glm::vec3* ppos, float rScale, glm::mat4* outMatScale);
void LoadScaleMatrixVector(glm::vec3* ppos, glm::mat3* pmat, glm::vec3* pvecScale, glm::mat4* outMatScale);
void BuildRotateVectorsMatrix(const glm::vec3* pvec1, const glm::vec3* pvec2, glm::mat3* pmat);
void LoadRotateMatrixRad(float rad, const glm::vec3* pnormal, glm::mat3* pmat);
void CalculateSinCos(float rad, float* pgSin, float* pgCos);

extern glm::vec3 g_normalX;
extern glm::vec3 g_normalY;
extern glm::vec3 g_normalZ;
extern glm::vec3 g_vecZero;