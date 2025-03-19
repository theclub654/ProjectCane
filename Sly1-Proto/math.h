#pragma once
#include "glm/vec3.hpp"
#include "glm/mat3x3.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"

void LoadMatrixFromPosRot(glm::vec3 &ppos, glm::mat3 &pmat, glm::mat4 &pmatDst);
void LoadMatrixFromPosRotScale(glm::vec3 &vecPos, glm::mat3 &matRot, glm::vec3 &vecScale, glm::mat4 &pmat);
void LoadMatrixFromPosRotInverse(glm::vec3 &pposSrc, glm::mat3 &pmatSrc, glm::mat4 &pmatDst);
// Builds rotation matrix
void BuildRotateVectorsMatrix(glm::vec3 &vec1, glm::vec3 &vec2, glm::mat4 &mat);
void LoadRotateMatrixRad(float rad, glm::vec3& pnormal, glm::mat4& pmat);