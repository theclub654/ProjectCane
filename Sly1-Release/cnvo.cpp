#include "cnvo.h"
#include "bbmark.h"

CNVO* NewCnvo()
{
	return new CNVO{};
}

void InitCnvo(CNVO* pcnvo)
{
	InitSo(pcnvo);
	pcnvo->svBelt = 100.0;
}

void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis)
{
    LoadSoFromBrx(pcnvo, pbis);

    pcnvo->svuAvg = pbis->F32Read();
    pcnvo->svvAvg = pbis->F32Read();
}

void CloneCnvo(CNVO* pcnvo, CNVO* pcnvoBase)
{
    CloneSo(pcnvo, pcnvoBase);

    pcnvo->svBelt = pcnvoBase->svBelt;
    pcnvo->svuAvg = pcnvoBase->svuAvg;
    pcnvo->svvAvg = pcnvoBase->svvAvg;
}

void PostCnvoLoad(CNVO* pcnvo)
{
    PostAloLoad(pcnvo);

    MatchCnvoScrollerToBeltSpeed(pcnvo);
}

void AdjustCnvoXpVelocity(CNVO* pcnvo, XP* pxp, int ixpd)
{
    glm::vec3 beltDir = glm::vec3(pcnvo->xf.matWorld[1]); // world Y axis
    glm::vec3 normal  = glm::vec3(pxp->normal);

    // Direction perpendicular to both belt axis and contact normal
    glm::vec3 vAdjust = glm::cross(beltDir, normal);

    float mag = glm::length(vAdjust);

    if (mag >= 0.0001f) 
    {
        vAdjust *= -(pcnvo->svBelt / mag);
        pxp->axpd[ixpd].v -= vAdjust;
    }
}

int GetCnvoSize()
{
	return sizeof(CNVO);
}

void SetCnvoBeltSpeed(CNVO* pcnvo, float svBelt)
{
    if (svBelt != pcnvo->svBelt)
    {
        pcnvo->svBelt = svBelt;
        InvalidateSwXpForObject(pcnvo->psw, pcnvo, 7);
        MatchCnvoScrollerToBeltSpeed(pcnvo);
        ResolveAlo(pcnvo);
    }
}

void MatchCnvoScrollerToBeltSpeed(CNVO* pcnvo)
{
    const int count = pcnvo->globset.apsaa.size();
    if (count <= 0) return;

    const float du = -(pcnvo->svBelt * pcnvo->svuAvg);
    const float dv = -(pcnvo->svBelt * pcnvo->svvAvg);

    for (int i = 0; i < count; ++i)
    {
        SCROLLER* saa = reinterpret_cast<SCROLLER*>(pcnvo->globset.apsaa[i]);
        if (!saa) continue;
        
        if (saa->saak == SAAK_Scroller)
        {
            saa->svu = du;
            saa->svv = dv;
            return;
        }
    }
}

void DeleteCnvo(CNVO *pcnvo)
{
	delete pcnvo;
}
