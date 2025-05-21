#include "binoc.h"

void StartupBinoc(BINOC *pbinoc)
{
	pbinoc->pvtbinoc = &g_vtbinoc;
}

void InitBinoc(BINOC* pbinoc, BLOTK blotk)
{
	pbinoc->dx = 640.0;
	pbinoc->dy = 492.8;
	pbinoc->svch = 15.0;

	InitBlot(pbinoc, blotk);
}

void PostBinocLoad(BINOC* pbinoc)
{
	PostBlotLoad(pbinoc);
}

void OnBinocActive(BINOC* pbinoc, int fActive)
{

}

void UpdateBinocActive(BINOC* pbinoc, JOY* pjoy)
{

}

void OnBinocReset(BINOC* pbinoc)
{

}

void OnBinocPush(BINOC* pbinoc)
{

}

void OnBinocPop(BINOC* pbinoc)
{

}

void SetBinocAchzDraw(BINOC* pbinoc, char* pchz)
{

}

float DtAppearBinoc(BINOC* pbinoc)
{
	return 0.0f;
}

float DtDisappearBinoc(BINOC* pbinoc)
{
	return 0.0f;
}

void SetBinocBlots(BINOC* pbinoc, BLOTS blots)
{

}

void DrawBinoc(BINOC* pbinoc)
{

}

SCAN*NewScan()
{
	return new SCAN{};
}

int GetScanSize()
{
	return sizeof(SCAN);
}

void CloneScan(SCAN* pscan, SCAN* pscanBase)
{
	ClonePnt(pscan, pscanBase);
	
	pscan->tbidCaption = pscanBase->tbidCaption;
}

void DeleteScan(SCAN* pscan)
{
	delete pscan;
}

BINOC g_binoc;