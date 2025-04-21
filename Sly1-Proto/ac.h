#pragma once
#include "lo.h"

enum ACVK
{
    ACVK_Nil = -1,
    ACVK_Component = 0,
    ACVK_Bezier = 1,
    ACVK_Blend = 2,
    ACVK_Max = 3
};
enum ACGK
{
    ACGK_Nil = -1,
    ACGK_Bezier = 0,
    ACGK_BezierWeighted = 1,
    ACGK_Linear = 2,
    ACGK_BlendTwist = 3,
    ACGK_BlendPose = 4,
    ACGK_Max = 5
};

struct KVB
{
    float t;
    glm::vec3 vec;
    glm::vec3 dvecIn;
    glm::vec3 dvecOut;
};
struct ACP
{
    union
    {
        struct VTACP* pvtacp;
        struct VTACPC* pvtacpc;
        struct VTACPB* pvtacpb;
        struct VTACPBL* pvtacpbl;
    };
    ACVK acvk;
    int fContiguous;
};

struct ACG
{
    union
    {
        struct VTACG* pvtacg;
        struct VTACGB* pvtacgb;
        struct VTACGBW* pvtacgbw;
        struct VTACGL* pvtacgl;
        struct VTACGBLT* pvtacgblt;
        struct VTACGBLP* pvtacgblp;
    };
    ACGK acgk;
};

struct ACPC : public ACP
{
    glm::vec3 posDefault;
    //std::vector <ACG> apacg;
    std::shared_ptr <ACG> apacg[3];
};

struct ACPB : public ACP
{
    int ckvb;
    std::vector <KVB> akvb;
};
 
std::shared_ptr <ACP> PacpNew(ACVK acvk);
void LoadAcpcFromBrx(std::shared_ptr <ACPC> pacpc, CBinaryInputStream* pbis);
void LoadAcpbFromBrx(std::shared_ptr <ACPB> pacpb, CBinaryInputStream* pbis);

struct VTACP {
    void(*pfnEvaluateAcp) = nullptr;
    void(*pfnLoadAcpFromBrx)() = nullptr;
    void(*pfnGetAcpTimes) = nullptr;
};

struct VTACPC {
    void(*pfnEvaluateAcpc) = nullptr;
    void(*pfnLoadAcpcFromBrx)(std::shared_ptr <ACPC>, CBinaryInputStream*) = LoadAcpcFromBrx;
    void(*pfnGetAcpcTimes) = nullptr;
};

struct VTACPB {
    void(*pfnEvaluateAcpb) = nullptr;
    void(*pfnLoadAcpbFromBrx)(std::shared_ptr <ACPB>, CBinaryInputStream*) = LoadAcpbFromBrx;
    void(*pfnGetAcpbTimes) = nullptr;
};

struct VTACPBL {
    void(*pfnEvaluateAcpbl) = nullptr;
    void(*pfnLoadAcpFromBrx) = nullptr;
    void(*pfnGetAcpTimes) = nullptr;
};

extern VTACPB g_vtacpb;
extern VTACPC g_vtacpc;

struct ACR
{
    union
    {
        struct VTACR* pvtacr;
        struct VTACRC* pvtacrc;
        struct VTACRB* pvtacrb;
        struct VTACRBL* pvtacrbl;
    };
    ACVK acvk;
    int fContiguous;
};

struct ACRC : public ACR
{
    glm::vec3 eulDefault;
    std::shared_ptr <ACG> apacg[3];
};

struct ACRB : public ACR
{
    int ckvb;
    std::vector <KVB> akvb;
};

std::shared_ptr <ACR> PacrNew(ACVK acvk);
void LoadAcrcFromBrx(std::shared_ptr <ACRC> pacrc, CBinaryInputStream* pbis);
void LoadAcrbFromBrx(std::shared_ptr <ACRB> pacrb, CBinaryInputStream* pbis);

struct VTACR {
    void(*pfnEvaluateAcr) = nullptr;
    void(*pfnEvaluateAcrEul) = nullptr;
    void(*pfnLoadAcrFromBrx) = nullptr;
    void(*pfnGetAcrTimes) = nullptr;
};

struct VTACRC {
    void(*pfnEvaluateAcrc) = nullptr;
    void(*pfnEvaluateAcrcEul) = nullptr;
    void(*pfnLoadAcrcFromBrx)(std::shared_ptr <ACRC>, CBinaryInputStream*) = LoadAcrcFromBrx;
    void(*pfnGetAcrcTimes) = nullptr;
};

struct VTACRBL {
    void(*pfnEvaluateAcrbl) = nullptr;
    void(*pfnEvaluateAcrEul) = nullptr;
    void(*pfnLoadAcrFromBrx) = nullptr;
    void(*pfnGetAcrTimes) = nullptr;
};

struct VTACRB {
    void(*pfnEvaluateAcrb) = nullptr;
    void(*pfnEvaluateAcrbEul) = nullptr;
    void(*pfnLoadAcrbFromBrx)(std::shared_ptr <ACRB>, CBinaryInputStream*) = LoadAcrbFromBrx;
    void(*pfnGetAcrbTimes) = nullptr;
};

extern VTACRB g_vtacrb;
extern VTACRC g_vtacrc;

struct ACS
{
    union
    {
        struct VTACS* pvtacs;
        struct VTACSC* pvtacsc;
        struct VTACSB* pvtacsb;
    };
    ACVK acvk;
};

struct ACSC : public ACS
{
    glm::vec3 vecDefault;
    std::shared_ptr <ACG> apacg[3];
};

struct ACSB : public ACS
{
    int ckvb;
    std::vector <KVB> akvb;
};

std::shared_ptr <ACS> PacsNew(ACVK acvk);
void LoadAcscFromBrx(std::shared_ptr <ACSC> pacsc, CBinaryInputStream* pbis);
void LoadAcsbFromBrx(std::shared_ptr <ACSB> pacsb, CBinaryInputStream* pbis);

struct VTACS {
    void(*pfnEvaluateAcs) = nullptr;
    void(*pfnLoadAcsFromBrx) = nullptr;
    void(*pfnGetAcsTimes) = nullptr;
};

struct VTACSB {
    void(*pfnEvaluateAcsb) = nullptr;
    void(*pfnLoadAcsbFromBrx)(std::shared_ptr <ACSB>, CBinaryInputStream*) = LoadAcsbFromBrx;
    void(*pfnGetAcsbTimes) = nullptr;
};

struct VTACSC {
    void(*pfnEvaluateAcsc) = nullptr;
    void(*pfnLoadAcscFromBrx)(std::shared_ptr <ACSC>, CBinaryInputStream*) = LoadAcscFromBrx;
    void(*pfnGetAcscTimes) = nullptr;
};

extern VTACSC g_vtacsc;
extern VTACSB g_vtacsb;

enum KGBTK
{
    KGBTK_Nil = -1,
    KGBTK_Global = 0,
    KGBTK_Fixed = 1,
    KGBTK_Linear = 2,
    KGBTK_Flat = 3,
    KGBTK_Smooth = 4,
    KGBTK_Step = 5,
    KGBTK_Slow = 6,
    KGBTK_Fast = 7,
    KGBTK_Clamped = 8,
    KGBTK_Max = 9
};

struct KGBT
{
    KGBTK kgbtk;
    float gSlope;
};

struct KGB
{
    float t;
    float g;
    struct KGBT kgbtIn;
    struct KGBT kgbtOut;
};

struct KGBWT
{
    KGBTK kgbtk;
    float dt;
    float g;
};

struct KGBW
{
    float t;
    float g;
    KGBWT kgbwtIn;
    KGBWT kgbwtOut;
};

struct KGL
{
    float t;
    float g;
};

struct ACGB : public ACG
{
    int ckgb;
    std::vector <KGB> akgb;
};

struct ACGBW : public ACG
{
    int ckgbw;
    std::vector <KGBW> akgbw;
};

struct ACGL : public ACG
{
    int ckgl;
    std::vector <KGL> akgl;
};

std::shared_ptr <ACG> PacgNew(ACGK acgk);
void LoadAcgbFromBrx(std::shared_ptr <ACGB> pacgb, CBinaryInputStream* pbis);
void LoadAcgbwFromBrx(std::shared_ptr <ACGBW> pacgbw, CBinaryInputStream* pbis);
void LoadAcglFromBrx(std::shared_ptr <ACGL> pacgl, CBinaryInputStream* pbis);

struct VTACGBLP {
    void(*pfnEvaluateAcgblp) = nullptr;
    void(*pfnLoadAcgFromBrx) = nullptr;
    void(*pfnGetAcgTimes) = nullptr;
};

struct VTACG {
    void(*pfnEvaluateAcg) = nullptr;
    void(*pfnLoadAcgFromBrx) = nullptr;
    void(*pfnGetAcgTimes) = nullptr;
};

struct VTACGB {
    void(*pfnEvaluateAcgb) = nullptr;
    void(*pfnLoadAcgbFromBrx)(std::shared_ptr <ACGB>, CBinaryInputStream*) = LoadAcgbFromBrx;
    void(*pfnGetAcgbTimes) = nullptr;
};

struct VTACGL {
    void(*pfnEvaluateAcgl) = nullptr;
    void(*pfnLoadAcglFromBrx)(std::shared_ptr <ACGL>, CBinaryInputStream*) = LoadAcglFromBrx;
    void(*pfnGetAcglTimes) = nullptr;
};

struct VTACGBW {
    void(*pfnEvaluateAcgbw) = nullptr;
    void(*pfnLoadAcgbwFromBrx)(std::shared_ptr <ACGBW>, CBinaryInputStream*) = LoadAcgbwFromBrx;
    void(*pfnGetAcgbwTimes) = nullptr;
};

struct VTACGBLT {
    void(*pfnEvaluateAcgblt) = nullptr;
    void(*pfnLoadAcgFromBrx) = nullptr;
    void(*pfnGetAcgTimes) = nullptr;
};

extern VTACGB g_vtacgb;
extern VTACGBW g_vtacgbw;
extern VTACGL g_vtacgl;
extern VTACGBLT g_vtacgblt;
extern VTACGBLP g_vtacgblp;

void LoadAkvbFromBrx(int* pckvb, std::vector <KVB>& pakvb, CBinaryInputStream* pbis);
void LoadApacgFromBrx(std::shared_ptr<ACG>(&apacg)[3], glm::vec3& pvecDefault, CBinaryInputStream* pbis);