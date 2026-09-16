#pragma once
#include "lo.h"

float SBezierPosLength(float dtSeg, float tSeg, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1);
void TesselateBezier(float dtSeg, float tStart, float tEnd, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1, int cpos, glm::vec3* apos);
void EvaluateBezierPos(float dtSeg, float tSeg, float svt, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1, glm::vec3* ppos, glm::vec3* pv, glm::vec3* pdv);
void EvaluateBezierWeightedFloat(float dtSeg, float tSeg, float svt, float g0, float dt0, float gCP0, float g1, float dt1, float gCP1, float* pg, float* pdg, float* pddg);
void EvaluateBezierFloat(float dtSeg, float tSeg, float svt, float g0, float gSlope0, float g1, float gSlope1, float* pg, float* pdg, float* pddg);
void EvaluateBezierMat(float dtSeg, float tSeg, float svt, const glm::mat3& mat0, const glm::vec3& w0, const glm::mat3& mat1, const glm::vec3& w1, glm::mat3* outMat, glm::vec3* outW, glm::vec3* outDW);
void LimitBezierMulti(int c, float* xValues, double xAccelerationMax, float* yValues, double yAccelerationMax, float* zValues, double zAccelerationMax, float* pdtMin);