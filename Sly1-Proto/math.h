#pragma once
#include "glm/vec3.hpp"
#include "glm/mat3x3.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtx/compatibility.hpp"

void LoadMatrixFromPosRot(glm::vec3 &ppos, glm::mat3 &pmat, glm::mat4 &pmatDst);
void LoadMatrixFromPosRotScale(glm::vec3 &vecPos, glm::mat3 &matRot, glm::vec3 &vecScale, glm::mat4 &pmat);
void LoadMatrixFromPosRotInverse(glm::vec3 &pposSrc, glm::mat3 &pmatSrc, glm::mat4 &pmatDst);
void BuildOrthonormalMatrixZ(glm::vec3 &pvecX, glm::vec3 &pvecZ, glm::mat4 &pmat);
// Builds rotation matrix
void BuildRotateVectorsMatrix(const glm::vec3& vec1, const glm::vec3& vec2, glm::mat3& outMatrix);
void LoadRotateMatrixRad(float rad, glm::vec3& pnormal, glm::mat3& pmat);
void DecomposeRotateMatrixEuler(const glm::mat3& mat, glm::vec3& eulOut);
void LoadRotateMatrixEuler(const glm::vec3& eul, glm::mat3* outMat);

extern glm::vec3 g_normalX;
extern glm::vec3 g_normalY;
extern glm::vec3 g_normalZ;