#pragma once
#include "cplcy.h"

CM*  NewCm();
// Initialize camera object
void InitCm(CM* pcm); // NOT FINISHED
int  GetCmSize();
void CloneCm(CM* pcm, CM* pcmBase);
void RecalcCmFrustrum(CM* pcm);
void BuildProjectionMatrix(float *fov, float *width, float *height, float *near, float *far, glm::mat4 &pmat);
void BuildSimpleProjectionMatrix(float rx, float ry, float dxOffset, float dyOffset, float sNear, float sFar, glm::mat4 &pmat);
void SetSwCameraFov(SW* psw, float radFOV);
void SetSwCameraNearClip(SW* psw, float sNearClip);
void SetSwCameraFarClip(SW* psw, float sFarClip);
void SetSwCameraNearFog(SW* psw, float sNearFog);
void SetSwCameraFarFog(SW* psw, float sFarFog);
void SetSwCameraUFogMax(SW* psw, float uFogMax);
void SetSwCameraRgbaFog(SW* psw, RGBA prgbaFog);
void SetSwCameraMrdRatio(SW* psw, float rMRD);
void*GetCmPos(CM* pcm);
void SetCmPos(CM* pcm, glm::vec3* ppos);
void*GetCmMat(CM* pcm);
void SetCmMat(CM* pcm, glm::mat3* pmat);
void SetCmPosMat(CM* pcm, glm::vec3 *ppos, glm::mat3 *pmat);
void*GetCmFov(CM* pcm);
void SetCmFov(CM* pcm, float radFOV);
void*GetCmNearClip(CM* pcm);
void SetCmNearClip(CM* pcm, float sNearClip);
void*GetCmFarClip(CM* pcm);
void SetCmFarClip(CM* pcm, float sFarClip);
void*GetCmNearFog(CM* pcm);
void SetCmNearFog(CM* pcm, float sNearFog);
void*GetCmFarFog(CM* pcm);
void SetCmFarFog(CM* pcm, float sFarFog);
void*GetCmUFogMax(CM* pcm);
void SetCmUFogMax(CM* pcm, float uFogMax);
void*GetCmRgbaFog(CM* pcm);
void SetCmRgbaFog(CM* pcm, RGBA prgbaFog);
void*GetCmMrdRatio(CM* pcm);
void SetCmMrdRatio(CM* pcm, float rMRD);
// Builds lookat matrix
void BuildLookAt(glm::vec3 &posEye, glm::vec3 &directionEye, glm::vec3 &upEye, glm::mat4 &pmatLookAt);
void SetupCm(CM *pcm);
// Combines a LookAt Matrix and Projection Matrix
void CombineEyeLookAtProj(const glm::vec3& eyePos, const glm::mat3& lookAt, const glm::mat4& proj, glm::mat4& pmat);
// Updates a camera matrix
void UpdateCmMat4(CM* pcm);
// Returns whether a object is in the camera view or not 
bool FInsideCmMrd(CM* pcm, float sRadius, float sMRD, float *puAlpha);
void DeleteCm(CM *pcm);