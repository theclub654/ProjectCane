#pragma once
#include "lo.h"

struct SMS
{
	OID oid;
	union
	{
		OID oidNext;
		int ismsNext;
	};
};
struct SMT
{
	int fAseg;
	struct ASEG* paseg;
	int ismsFrom;
	int ismsTo;
	int grfsmt;
	float gProbability;
};

class SMA : public BASIC
{
	public:
	DLE dleSm;
	DLE dleSw;
	struct SM* psm;
	class ALO* paloRoot;
	int grfapl;
	struct ASEGA* pasegaCur;
	int ismsCur;
	int ismsNext;
	int ismsGoal;
	struct SMT* psmtCur;
	float svtLocal;
	std::shared_ptr <MQ> pmqFirst;
};

class SM : public LO
{
	public:
	int csms;
	std::vector <SMS> asms;
	int csmt;
	std::vector <SMT> asmt;
	int fDefault;
	DL dlSma;
};

SM*  NewSm();
int  GetSmSize();
void LoadSmFromBrx(SM *psm, CBinaryInputStream *pbis);
void CloneSm(SM* psm, SM* psmBase);
void SetSmFDefault(SM* psm, int fDefault);
void*GetSmFDefault(SM* psm);
void*GetSmaPaloRoot(SMA* psma);
void*GetSmaSvt(SMA* psma);
void*GetSmaAsegaCur(SMA* psma);
void*GetSmaPsm(SMA* psma);
void PostSmLoad(SM* psm);
void PostSmLoadCallback(SM* psm, MSGID msgid, void* pvData);
SMA* PsmaApplySm(SM* psm, ALO* paloRoot, OID oidInitialState, GRFAPL grfapl);
SMA* PsmaFindSm(SM* psm, ALO* paloRoot);
int  IsmsFindSmOptional(SM* psm, OID oid);
int  IsmsFindSmRequired(SM* psm, OID oid);
int  OidFromSmIsms(SM* psm, int isms);
void RetractSma(SMA* psma);
void SetSmaGoal(SMA* psma, OID oid);
void GetSmaGoal(const SMA* psma, OID* poid);
void GetSmaCur(SMA* psma, OID* poid);
void GetSmaNext(SMA* psma, OID* poid);
void SetSmaSvt(SMA* psma, float svt);
void SeekSma(SMA* psma, OID oid);
void ChooseSmaTransition(SMA* psma);
void EndSmaTransition(SMA* psma);
void HandleSmaMessage(SMA* psma, MSGID msgid, void* pv);
void SkipSma(SMA* psma, float dtSkip);
void SendSmaMessage(SMA* psma, MSGID msgid, void* pv);
void SubscribeSmaStruct(SMA* psma, LO* plo);
void NotifySmaSpliceOnEnterState(SMA* psma, int ismsFrom, int ismsTo);
void DeleteSm(SM* psm);
