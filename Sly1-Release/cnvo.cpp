#include "cnvo.h"

CNVO* NewCnvo()
{
	return new CNVO{};
}

void InitCnvo(CNVO* pcnvo)
{
	InitSo(pcnvo);
	pcnvo->svBelt = 100.0;
}

int GetCnvoSize()
{
	return sizeof(CNVO);
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
	pcnvo->svvAvg = pcnvo->svvAvg;
}

void MatchCnvoScrollerToBeltSpeed(CNVO* cnvo)
{
    if (!cnvo) return;

    const int count = cnvo->globset.apsaa.size();
    if (count <= 0) return;

    SAA* list = cnvo->globset.apsaa[0];
    if (!list) return;

    const float du = -(cnvo->svBelt * cnvo->svuAvg);
    const float dv = -(cnvo->svBelt * cnvo->svvAvg);

    for (int i = 0; i < count; ++i)
    {
        SCROLLER* saa = (SCROLLER*)&list[i];
        if (!saa) continue;
        
        if (saa->saak == SAAK_Scroller)
        {
            saa->svu = du;
            saa->svv = dv;
            return;
        }
    }
}

void SetCnvoBeltSpeed(CNVO* pcnvo, float svBelt)
{
    if (svBelt != pcnvo->svBelt) 
    {
        pcnvo->svBelt = svBelt;
        //InvalidateSwXpForObject(pcnvo->psw, pcnvo, 7);
        MatchCnvoScrollerToBeltSpeed(pcnvo);
        ResolveAlo(pcnvo);
    }
}

void PostCnvoLoad(CNVO* pcnvo)
{
	PostAloLoad(pcnvo);
	MatchCnvoScrollerToBeltSpeed(pcnvo);
}

void DeleteCnvo(CNVO *pcnvo)
{
	delete pcnvo;
}
