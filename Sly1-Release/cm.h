#pragma once
enum LOOKK;
struct CPMAN;
struct CPLOOK;
struct CPALIGN;
struct CPASEG;
struct CPTN;
struct SBI;

#include "cplcy.h"
#include "cpman.h"
#include "cplook.h"
#include "cpalign.h"
#include "cpaseg.h"
#include "cptn.h"

enum CMLK
{
    CMLK_Nil = -1,
    CMLK_Grfzon = 0,
    CMLK_Mrd = 1,
    CMLK_Max = 2
};

struct FRUSTUM 
{
    std::array <glm::vec4, 6> planes; // x,y,z = normal, w = distance
};

// Camera Object
class CM : public LO
{
    public:
    // Projection and lookAt matrix combined
    glm::mat4 matWorldToClip;
    // Camera psoition
    glm::vec3 pos;
    float yaw;
    float pitch;
    glm::vec4 anormalFrustrumTranspose[3];
    FRUSTUM frustum;
    glm::mat3 mat;
    float rMRDAdjust;
    // Camera projection 
    glm::mat4 matProj;
    glm::mat4 matClipToWorld;
    glm::vec3 anormalFrustrum[4];
    float rMRD;
    // Camera field of view
    float radFOV;
    float radFOVTarget;
    float svRadFOV;
    SMPA smpaRadFOV;
    // Camera Aspect Ratio
    float rAspect;
    float sNearClip;
    float sFarClip;
    float sRadiusNearClip;
    float xScreenRange;
    float yScreenRange;
    float sNearFog;
    float sFarFog;
    float uFogMax;
    glm::vec4 rgbaFog;
    FGFN fgfn;
    float tJolt;
    GRFZON grfzon;
    int fCutNext;
    int fCut;
    int fRadCut;
    float radCut;
    int fDisplaced;
    float uPanProgress;
    float uTiltProgress;
    float uSProgress;
    glm::vec3 dposCenter;
    glm::vec3 vCenter;
    glm::vec3 dposAdjust;
    glm::vec3 vAdjust;
    glm::vec3 dposFocus;
    glm::vec3 vFocus;
    glm::vec3 posScreen;
    glm::vec3 vScreen;
    float swPanPos;
    float swTiltPos;
    float sv;
    float swPanMat;
    float swTiltMat;
    glm::vec3 posCenterPrev;
    glm::vec3 posClear;
    glm::mat3 matClear;
    SO* psoFocusPrev;
    CPDEFI cpdefiPrev;
    int cpaloFade;
    class ALO* apaloFade[8];
    float tActivateCplcy;
    glm::mat3 matRotateToCam;
    glm::mat3 matRotateTiltToCam;
    int ccpr;
    CPR acpr[8];
    CPMAN cpman;
    CPLOOK cplook;
    CPALIGN cpalign;
    CPASEG cpaseg;
    CPTN cptn;
};

void CalcCmAdjust(CM* pcm, SO* psoFocus, CPDEFI* pcpdefi, glm::vec3* pdpos);
void SpringCm(CM* pcm, glm::vec3* ppos, glm::vec3* pv, const glm::vec3* pposTarget, float rSpring, float rDamping, float dt);
void AdjustCmCpdefi(CM* pcm, JT* pjtFocus, float dt, CPDEFI* pcpdefi);
void ResetCmAdjust(CM* pcm, SO* psoFocus, CPDEFI* pcpdefi);
void GetCmCpdefi(CM* pcm, SO* psoFocus, float dt, CPDEFI* pcpdefi);
void RecalcCm(CM* pcm);
CM*  NewCm();
// Initialize camera object
void InitCm(CM* pcm); // NOT FINISHED
int  GetCmSize();
void CloneCm(CM* pcm, CM* pcmBase);
void BuildProjectionMatrix(float fov, float aspectRatio, float near, float far, glm::mat4& pmat);
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
void*GetCmFov(CM* pcm);
void SetCmFov(CM* pcm, float radFOV);
void*GetCmRadFOVTarget(CM* pcm);
void SetCmRadFOVTarget(CM* pcm, float radFOVTarget);
void*GetCmSmpaRadFOV(CM* pcm);
void SetCmSmpaRadFOV(CM* pcm, SMPA smpaRadFOV);
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
void ResetCm(CM* pcm, int fResetLookat);
void ClearCmFadeObjects(CM* pcm);
void AddCmFadeObject(CM* pcm, ALO* palo);
void RemoveCmFadeObject(CM* pcm, ALO* palo);
int  FFilterCamera(void* pv, SO* pso);
void UpdateCmFade(CM* pcm);
void UpdateCmLast(CM* pcm, int fClearCut, float dt);
void SetupCmRotateToCam(CM* pcm);
void ConvertCmScreenToWorld(CM* pcm, const glm::vec3* pposScreen, glm::vec3* pposWorld);
void ConvertCmWorldToScreen(CM* pcm, const glm::vec3* pposWorld, glm::vec3* pposScreen);
void SetupCm(CM* pcm);
void CombineEyeLookAtProj(const glm::vec3& pposEye, const glm::mat3& pmatLookAt, const glm::mat4& pmatProj, glm::mat4& pmatOut);
// Makes frustum
void BuildFrustrum(const glm::mat3& pmatLookAt, float rx, float ry, glm::vec3* anormalFrustrum);
// Updates a camera matrix
void UpdateCmMat4(CM* pcm);
void SetCmPosMat(CM* pcm, const glm::vec3* ppos, const glm::mat3* pmat);
void SetCmLookAt(CM* pcm, const glm::vec3* pposEye, const glm::vec3* pposCenter);
void ConvertWorldToCylindVelocity(const glm::vec3* pposCenter, const glm::vec3* ppos, const glm::vec3* pv, float* pswPan, float* pswTilt, float* psv);
void ConvertCylindToWorldVelocity(const glm::vec3* pposCenter, const glm::vec3* ppos, float swPan, float swTilt, float sv, glm::vec3* pv);
void ResetCmLookAtSmooth(CM* pcm, const glm::vec3* pposCenter);
void SetCmLookAtSmooth(CM* pcm, int grflas, const glm::vec3* pposEye, const glm::vec3* pposCenter, SO* psoFocus, float rSpring, float rDamping, float rLimit, float xScreenPref, float yScreenPref, float dt);
void AdjustCmJoy(CM* pcm, JOY* pjoy, JOYID joyid, float* prad);
void SetCmPolicy(CM* pcm, CPP cpp, CPLCY* pcplcy, SO* psoFocus, void* pv);
void RevokeCmPolicy(CM* pcm, GRFRCP grfrcp, CPP cpp, CPLCY* pcplcy, SO* psoFocus, void* pv);
bool FInsideCmMrd(const CM* pcm, const glm::vec3& dpos, float sRadius, float sMRD, float& outAlpha);
void SetCmCut(CM* pcm, float* pradCut);
void SetCmCutNext(CM* pcm);
void CutCm(CM* pcm);
void AdaptCm(CM* pcm, const glm::vec3* pposCenter, float xScreen, float yScreen);
void SetCm(CM* pcm, const glm::vec3* pposCenter, const glm::vec3* pposEye, float xScreen, float yScreen);
int  FFindCmClearInterval(float duMin, int cpso, SO** apso, glm::vec3* pposFrom, glm::vec3* pposTo, float* puFrom, float* puTo);
void FindCmClosestClearPos(float sRadius, int cpso, SO** apso, glm::vec3* ppos, glm::vec3* pposClear);
void SquishCmEye(glm::vec3* pposEyePrev, float sdposMax, void* pvContext, int cpso, SO** apso, PFNSQUISHCMEYE pfnConstraint, float sRadius, const glm::vec3* pposEyeNext, glm::vec3* pposEyeClip);
bool FCmTunnelHome(void* pv, CM* pcm);
float SvSquishCmEyeConstraint(void* pvContext, float sRadius, const glm::vec3* pvEye, const SBI* psbi);
void ClipCmEye(CM* pcm, const glm::vec3* pposEyePrev, glm::vec3* pposEyeNext, glm::vec3* pposEyeClip);
void PushCmLookk(CM* pcm, LOOKK lookk);
LOOKK LookkPopCm(CM* pcm);
LOOKK LookkCurCm(CM* pcm);
void SetCmSniperFocus(CM* pcm, ALO* palo, float rScreen, float sRadius);
void JoltCm(CM* pcm);
bool FVisibleCmSphere(const CM* pcm, const glm::vec3& pos, float sRadius, float sMRD, float& outAlpha);
// Builds lookat matrix 
void BuildLookAt(glm::vec3 &posEye, glm::vec3 &directionEye, glm::vec3 &upEye, glm::mat4 &pmatLookAt);
void UnlockCm(int nParam);
glm::mat3 ConvertGameCameraBasisToGl(const glm::mat3& gameBasis);
glm::mat3 ConvertGlCameraBasisToGame(const glm::mat3& glBasis);
// Extract frustum planes from matrix
void ExtractFrustumPlanes(const glm::mat4& viewProj, FRUSTUM* pfrustum);
// Checks if a object is in camera frustum
bool SphereInFrustum(const FRUSTUM& frustum, const glm::vec3& center, float radius);
void DeleteCm(CM *pcm);

extern CMLK g_cmlk;
extern glm::vec3 g_posEyeDefault;
extern glm::vec3 g_posCenterDefault;
extern float R_SetCmDamping;
extern float R_SetCmSpring;
extern float s_acCmClearSamples[9];
extern glm::vec3 s_dposCmSquishMin;
extern glm::vec3 s_dposCmSquishMax;
extern float SV_CmSquishEyeSlack;
extern float S_CmSquishEye;
extern SMP s_smpSquishEye;
extern SMPA s_smpaRadFOV;
extern float g_uFogMax;
extern float g_sNearFog;
extern float g_sFarFog;
extern glm::vec4 g_rgbaFog;
