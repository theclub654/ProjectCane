#include "spaprops.h"
#include "cidinc.h"
#include "clock.h"
#include "stepguard.h"
#include "mgv.h"
#include "pnt.h"
#include <cstdio>
#include <intrin.h>

void BuildEopids()
{
	g_aeopid[0].otyp = OTYP_Cid;
	g_aeopid[0].grfeopid = 4;
	g_aeopid[0].optdat.pfnget = reinterpret_cast<PFNRAW>(GetBasicCid);

	g_aeopid[1].otyp = OTYP_Bool;
	g_aeopid[1].grfeopid = 1;
	g_aeopid[1].optdat.pvThunkFn = RefThunkBasicDERIVED_FROMQ;
	g_aeopid[1].optdat.crefReq = 1;

	g_aeopid[2].otyp = OTYP_Void;
	g_aeopid[2].grfeopid = 0x401;
	g_aeopid[2].optdat.pvThunkFnUser = RefThunkLoREMOVEFnUser;
	g_aeopid[2].optdat.pvThunkFn = RefThunkLoREMOVEFn;
	g_aeopid[2].optdat.crefReq = 0;

	g_aeopid[3].otyp = OTYP_Lm;
	g_aeopid[3].grfeopid = 0x100;
	g_aeopid[3].optdat.pfnsetUser = reinterpret_cast<PFNRAW>(SetLoSuckHideLimits);

	g_aeopid[4].otyp = OTYP_Void;
	g_aeopid[4].grfeopid = 1;
	g_aeopid[4].optdat.pvThunkFn = RefThunkLoADD;
	g_aeopid[4].optdat.crefReq = 0;

	g_aeopid[5].otyp = OTYP_Lo;
	g_aeopid[5].grfeopid = 1;
	g_aeopid[5].optdat.pvThunkFn = RefThunkLoCLONE;
	g_aeopid[5].optdat.crefReq = 2;

	g_aeopid[6].otyp = OTYP_Sw;
	g_aeopid[6].grfeopid = 2;
	g_aeopid[6].optdat.ibGet = static_cast<std::ptrdiff_t>(offsetof(LO, psw));

	g_aeopid[7].otyp = OTYP_Bool;
	g_aeopid[7].grfeopid = 4;
	g_aeopid[7].optdat.pfnget = reinterpret_cast<PFNRAW>(GetLoInWorld);

	g_aeopid[8].otyp = OTYP_Alo;
	g_aeopid[8].grfeopid = 2;
	g_aeopid[8].optdat.ibGet = static_cast<std::ptrdiff_t>(offsetof(LO, paloParent));

	g_aeopid[9].otyp = OTYP_Void;
	g_aeopid[9].grfeopid = 1;
	g_aeopid[9].optdat.pvThunkFn = RefThunkLoSETPARENT;
	g_aeopid[9].optdat.crefReq = 1;

	g_aeopid[10].otyp = OTYP_Bool;
	g_aeopid[10].grfeopid = 1;
	g_aeopid[10].optdat.pvThunkFn = RefThunkLoANCESTORQ;
	g_aeopid[10].optdat.crefReq = 1;

	g_aeopid[11].otyp = OTYP_Oid;
	g_aeopid[11].grfeopid = 2;
	g_aeopid[11].optdat.ibGet = static_cast<std::ptrdiff_t>(offsetof(LO, oid));

	g_aeopid[12].optdat.pfnget = (PFNRAW)GetLoOidProxy;
	g_aeopid[12].otyp = OTYP_Oid;
	g_aeopid[12].grfeopid = 4;

	g_aeopid[13].otyp = OTYP_Bool;
	g_aeopid[13].optdat.ibGet = 0;
	g_aeopid[13].optdat.ibSet = 1;
	g_aeopid[13].optdat.pvThunkFn = RefThunkLoMATCHES_NAMEQ;
	g_aeopid[13].grfeopid = 1;

	g_aeopid[14].otyp = OTYP_Void;
	g_aeopid[14].optdat.ibSet = 2;
	g_aeopid[14].grfeopid = 0x400;
	g_aeopid[14].optdat.pvThunkFnUser = RefThunkLoCUSTOM_BOOL_PROPERTY;

	g_aeopid[15].otyp = OTYP_Void;
	g_aeopid[15].optdat.ibSet = 2;
	g_aeopid[15].grfeopid = 0x400;
	g_aeopid[15].optdat.pvThunkFnUser = RefThunkLoCUSTOM_INT_PROPERTY;

	g_aeopid[16].otyp = OTYP_Void;
	g_aeopid[16].grfeopid = 0x400;
	g_aeopid[16].optdat.ibSet = 2;
	g_aeopid[16].optdat.pvThunkFnUser = RefThunkLoCUSTOM_FLOAT_PROPERTY;

	g_aeopid[17].otyp = OTYP_Void;
	g_aeopid[17].grfeopid = 0x400;
	g_aeopid[17].optdat.ibSet = 2;
	g_aeopid[17].optdat.pvThunkFnUser = RefThunkLoCUSTOM_CLQ_PROPERTY;

	g_aeopid[18].otyp = OTYP_Void;
	g_aeopid[18].grfeopid = 0x400;
	g_aeopid[18].optdat.ibSet = 2;
	g_aeopid[18].optdat.pvThunkFnUser = RefThunkLoCUSTOM_LM_PROPERTY;

	g_aeopid[19].otyp = OTYP_Void;
	g_aeopid[19].grfeopid = 0x400;
	g_aeopid[19].optdat.ibSet = 2;
	g_aeopid[19].optdat.pvThunkFnUser = RefThunkLoCUSTOM_OID_PROPERTY;

	g_aeopid[20].grfeopid = 0x400;
	g_aeopid[20].otyp = OTYP_Void;
	g_aeopid[20].optdat.ibSet = 2;
	g_aeopid[20].optdat.pvThunkFnUser = RefThunkLoCUSTOM_VECTOR_PROPERTY;

	g_aeopid[21].grfeopid = 256;
	g_aeopid[21].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloInitialVelocity);
	g_aeopid[21].otyp = OTYP_Vector;

	g_aeopid[22].grfeopid = 256;
	g_aeopid[22].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloInitialAngularVelocity);
	g_aeopid[22].otyp = OTYP_Vector;

	g_aeopid[23].grfeopid = 4224;
	g_aeopid[23].otyp = OTYP_Oid;
	g_aeopid[23].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ASEGD, oidAseg));
	g_aeopid[23].optdat.pfnensure = (PFNENSURE)PasegdEnsureAlo;

	g_aeopid[24].otyp = OTYP_Float;
	g_aeopid[24].grfeopid = 4224;
	g_aeopid[24].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ASEGD, tLocal));
	g_aeopid[24].optdat.pfnensure = (PFNENSURE)PasegdEnsureAlo;

	g_aeopid[25].otyp = OTYP_Float;
	g_aeopid[25].grfeopid = 4224;
	g_aeopid[25].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ASEGD, svtLocal));
	g_aeopid[25].optdat.pfnensure = (PFNENSURE)PasegdEnsureAlo;

	g_aeopid[26].grfeopid = 4224;
	g_aeopid[26].otyp = OTYP_Rpk;
	g_aeopid[26].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ASEGD, iak));
	g_aeopid[26].optdat.pfnensure = (PFNENSURE)PasegdEnsureAlo;

	g_aeopid[27].otyp = OTYP_Void;
	g_aeopid[27].grfeopid = 1;
	g_aeopid[27].optdat.pvThunkFn = RefThunkAloINVALIDATE_LIGHTING;
	g_aeopid[27].optdat.crefReq = 0;

	g_aeopid[28].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloFrozen;
	g_aeopid[28].otyp = OTYP_Bool;
	g_aeopid[28].grfeopid = 4;

	g_aeopid[29].otyp = OTYP_Vector;
	g_aeopid[29].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALO, xf) + offsetof(XF, pos));
	g_aeopid[29].grfeopid = 66;
	g_aeopid[29].optdat.ibSet = static_cast<std::intptr_t>(offsetof(VTALO, pfnTranslateAloToPos));

	g_aeopid[30].otyp = OTYP_Vector;
	g_aeopid[30].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALO, posOrig));
	g_aeopid[30].grfeopid = 18;
	g_aeopid[30].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ALO, posOrig));

	g_aeopid[31].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALO, xf) + offsetof(XF, posWorld));
	g_aeopid[31].otyp = OTYP_Vector;
	g_aeopid[31].grfeopid = 2;

	g_aeopid[32].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALO, xf) + offsetof(XF, mat));
	g_aeopid[32].otyp = OTYP_Matrix;
	g_aeopid[32].grfeopid = 66;
	g_aeopid[32].optdat.ibSet = static_cast<std::intptr_t>(offsetof(VTALO, pfnRotateAloToMat));

	g_aeopid[33].otyp = OTYP_Matrix;
	g_aeopid[33].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALO, matOrig));
	g_aeopid[33].grfeopid = 18;
	g_aeopid[33].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ALO, matOrig));

	g_aeopid[34].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALO, xf) + offsetof(XF, matWorld));
	g_aeopid[34].otyp = OTYP_Matrix;
	g_aeopid[34].grfeopid = 2;

	g_aeopid[35].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloEuler;
	g_aeopid[35].otyp = OTYP_Vector;
	g_aeopid[35].grfeopid = 292;
	g_aeopid[35].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloEuler);
	g_aeopid[35].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloEuler);

	g_aeopid[36].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloVelocityLocal;
	g_aeopid[36].otyp = OTYP_Vector;
	g_aeopid[36].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloVelocityLocal);
	g_aeopid[36].grfeopid = 36;

	g_aeopid[37].grfeopid = 66;
	g_aeopid[37].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALO, xf) + offsetof(XF, w));
	g_aeopid[37].otyp = OTYP_Vector;
	g_aeopid[37].optdat.ibSet = static_cast<std::intptr_t>(offsetof(VTALO, pfnSetAloAngularVelocityVec));

	g_aeopid[38].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALO, xf) + offsetof(XF, dv));
	g_aeopid[38].otyp = OTYP_Vector;
	g_aeopid[38].grfeopid = 2;

	g_aeopid[39].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALO, xf) + offsetof(XF, dw));
	g_aeopid[39].otyp = OTYP_Vector;
	g_aeopid[39].grfeopid = 2;

	g_aeopid[40].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALO, paloRoot));
	g_aeopid[40].otyp = OTYP_Alo;
	g_aeopid[40].grfeopid = 2;

	g_aeopid[41].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloFastShadowRadius;
	g_aeopid[41].otyp = OTYP_Float;
	g_aeopid[41].grfeopid = 292;
	g_aeopid[41].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloFastShadowRadius);
	g_aeopid[41].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloFastShadowRadius);

	g_aeopid[42].otyp = OTYP_Float;
	g_aeopid[42].grfeopid = 292;
	g_aeopid[42].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloFastShadowDepth;
	g_aeopid[42].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloFastShadowDepth);
	g_aeopid[42].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloFastShadowDepth);

	g_aeopid[43].grfeopid = 292;
	g_aeopid[43].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloCastShadow;
	g_aeopid[43].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloCastShadow);
	g_aeopid[43].otyp = OTYP_Bool;
	g_aeopid[43].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloCastShadow);

	g_aeopid[44].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloShadowShader);
	g_aeopid[44].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloShadowShader);
	g_aeopid[44].otyp = OTYP_Oid;
	g_aeopid[44].grfeopid = 292;
	g_aeopid[44].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloShadowShader;

	g_aeopid[45].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloShadowNearRadius;
	g_aeopid[45].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloShadowNearRadius);
	g_aeopid[45].otyp = OTYP_Float;
	g_aeopid[45].grfeopid = 292;
	g_aeopid[45].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloShadowNearRadius);

	g_aeopid[46].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloShadowFarRadius);
	g_aeopid[46].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloShadowFarRadius);
	g_aeopid[46].otyp = OTYP_Float;
	g_aeopid[46].grfeopid = 292;
	g_aeopid[46].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloShadowFarRadius;

	g_aeopid[47].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloShadowNearCast);
	g_aeopid[47].otyp = OTYP_Float;
	g_aeopid[47].grfeopid = 292;
	g_aeopid[47].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloShadowNearCast;
	g_aeopid[47].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloShadowNearCast);

	g_aeopid[48].otyp = OTYP_Float;
	g_aeopid[48].grfeopid = 292;
	g_aeopid[48].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloShadowFarCast;
	g_aeopid[48].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloShadowFarCast);
	g_aeopid[48].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloShadowFarCast);

	g_aeopid[49].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloShadowConeAngle);
	g_aeopid[49].grfeopid = 292;
	g_aeopid[49].otyp = OTYP_Float;
	g_aeopid[49].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloShadowConeAngle);
	g_aeopid[49].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloShadowConeAngle;

	g_aeopid[50].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloShadowFrustrumUp);
	g_aeopid[50].grfeopid = 292;
	g_aeopid[50].otyp = OTYP_Vector;
	g_aeopid[50].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloShadowFrustrumUp);
	g_aeopid[50].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloShadowFrustrumUp;

	g_aeopid[51].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloDynamicShadowObject);
	g_aeopid[51].grfeopid = 256;
	g_aeopid[51].otyp = OTYP_Oid;

	g_aeopid[52].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloNoFreeze);
	g_aeopid[52].otyp = OTYP_Bool;
	g_aeopid[52].grfeopid = 288;
	g_aeopid[52].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloNoFreeze);

	g_aeopid[53].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloFreezeMode);
	g_aeopid[53].grfeopid = 292;
	g_aeopid[53].otyp = 4196;
	g_aeopid[53].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloFreezeMode);
	g_aeopid[53].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloFreezeMode;

	g_aeopid[54].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloRestorePosition);
	g_aeopid[54].grfeopid = 256;
	g_aeopid[54].otyp = OTYP_Bool;

	g_aeopid[55].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloRestorePositionAck);
	g_aeopid[55].grfeopid = 256;
	g_aeopid[55].otyp = OTYP_Mtlk;

	g_aeopid[56].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloPositionSpring);
	g_aeopid[56].grfeopid = 288;
	g_aeopid[56].otyp = OTYP_Float;
	g_aeopid[56].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloPositionSpring);

	g_aeopid[57].otyp = OTYP_Clq;
	g_aeopid[57].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloPositionSpringDetail);
	g_aeopid[57].grfeopid = 288;
	g_aeopid[57].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloPositionSpringDetail);

	g_aeopid[58].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloPositionDamping);
	g_aeopid[58].grfeopid = 288;
	g_aeopid[58].otyp = OTYP_Float;
	g_aeopid[58].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloPositionDamping);

	g_aeopid[59].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloPositionDampingDetail);
	g_aeopid[59].grfeopid = 288;
	g_aeopid[59].otyp = OTYP_Clq;
	g_aeopid[59].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloPositionDampingDetail);

	g_aeopid[60].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloRestoreRotation);
	g_aeopid[60].grfeopid = 256;
	g_aeopid[60].otyp = OTYP_Bool;

	g_aeopid[61].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloRestoreRotationAck);
	g_aeopid[61].otyp = OTYP_Mtlk;
	g_aeopid[61].grfeopid = 256;

	g_aeopid[62].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloActPriority);
	g_aeopid[62].grfeopid = 288;
	g_aeopid[62].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloActPriority);
	g_aeopid[62].otyp = OTYP_Int;

	g_aeopid[63].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloRotationSpring);
	g_aeopid[63].grfeopid = 288;
	g_aeopid[63].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloRotationSpring);
	g_aeopid[63].otyp = OTYP_Float;

	g_aeopid[64].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloRotationSpringDetail);
	g_aeopid[64].grfeopid = 288;
	g_aeopid[64].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloRotationSpringDetail);
	g_aeopid[64].otyp = OTYP_Clq;

	g_aeopid[65].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloRotationDamping);
	g_aeopid[65].grfeopid = 288;
	g_aeopid[65].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloRotationDamping);
	g_aeopid[65].otyp = OTYP_Float;

	g_aeopid[66].otyp = OTYP_Clq;
	g_aeopid[66].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloRotationDampingDetail);
	g_aeopid[66].grfeopid = 288;
	g_aeopid[66].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloRotationDampingDetail);

	g_aeopid[67].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloPositionSmooth);
	g_aeopid[67].grfeopid = 288;
	g_aeopid[67].otyp = OTYP_Float;
	g_aeopid[67].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloPositionSmooth);

	g_aeopid[68].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloPositionSmoothDetail);
	g_aeopid[68].grfeopid = 288;
	g_aeopid[68].otyp = OTYP_Smpa;
	g_aeopid[68].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloPositionSmoothDetail);

	g_aeopid[69].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloRotationSmooth);
	g_aeopid[69].grfeopid = 288;
	g_aeopid[69].otyp = OTYP_Float;
	g_aeopid[69].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloRotationSmooth);

	g_aeopid[70].otyp = OTYP_Smpa;
	g_aeopid[70].grfeopid = 288;
	g_aeopid[70].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloRotationSmoothDetail);
	g_aeopid[70].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloRotationSmoothDetail);

	g_aeopid[71].grfeopid = 288;
	g_aeopid[71].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloPositionSmoothMaxAccel);
	g_aeopid[71].otyp = OTYP_Float;
	g_aeopid[71].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloPositionSmoothMaxAccel);

	g_aeopid[72].grfeopid = 288;
	g_aeopid[72].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloRotationSmoothMaxAccel);
	g_aeopid[72].otyp = OTYP_Float;
	g_aeopid[72].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloRotationSmoothMaxAccel);

	g_aeopid[73].grfeopid = 256;
	g_aeopid[73].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloDefaultAckPos);
	g_aeopid[73].otyp = OTYP_Mtlk;

	g_aeopid[74].grfeopid = 256;
	g_aeopid[74].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloDefaultAckRot);
	g_aeopid[74].otyp = OTYP_Mtlk;

	g_aeopid[75].otyp = OTYP_Vector4;
	g_aeopid[75].grfeopid = 0;

	g_aeopid[76].otyp = OTYP_Mtlk;
	g_aeopid[76].grfeopid = 256;
	g_aeopid[76].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloLookAt);

	g_aeopid[77].otyp = OTYP_Float;
	g_aeopid[77].grfeopid = 292;
	g_aeopid[77].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloLookAtIgnore);
	g_aeopid[77].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloLookAtIgnore;
	g_aeopid[77].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloLookAtIgnore);

	g_aeopid[78].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloLookAtPanFunction;
	g_aeopid[78].otyp = OTYP_Clq;
	g_aeopid[78].grfeopid = 292;
	g_aeopid[78].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloLookAtPanFunction);
	g_aeopid[78].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloLookAtPanFunction);

	g_aeopid[79].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloLookAtPanLimits;
	g_aeopid[79].otyp = OTYP_Lm;
	g_aeopid[79].grfeopid = 292;
	g_aeopid[79].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloLookAtPanLimits);
	g_aeopid[79].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloLookAtPanLimits);

	g_aeopid[80].otyp = OTYP_Clq;
	g_aeopid[80].grfeopid = 292;
	g_aeopid[80].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloLookAtTiltFunction;
	g_aeopid[80].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloLookAtTiltFunction);
	g_aeopid[80].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloLookAtTiltFunction);

	g_aeopid[81].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloLookAtTiltLimits;
	g_aeopid[81].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloLookAtTiltLimits);
	g_aeopid[81].otyp = OTYP_Lm;
	g_aeopid[81].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloLookAtTiltLimits);
	g_aeopid[81].grfeopid = 292;

	g_aeopid[82].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloLookAtEnabledPriority;
	g_aeopid[82].otyp = OTYP_Int;
	g_aeopid[82].grfeopid = 292;
	g_aeopid[82].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloLookAtEnabledPriority);
	g_aeopid[82].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloLookAtEnabledPriority);

	g_aeopid[83].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloLookAtDisabledPriority;
	g_aeopid[83].otyp = OTYP_Int;
	g_aeopid[83].grfeopid = 292;
	g_aeopid[83].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloLookAtDisabledPriority);
	g_aeopid[83].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloLookAtDisabledPriority);

	g_aeopid[84].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloLookAtEnabledPriority;
	g_aeopid[84].otyp = OTYP_Oid;
	g_aeopid[84].grfeopid = 260;
	g_aeopid[84].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloLookAtDisabledPriority);

	g_aeopid[85].otyp = OTYP_Alo;
	g_aeopid[85].grfeopid = 36;
	g_aeopid[85].optdat.pvThunkFnUser = reinterpret_cast<PVTHUNKFN>(GetAloActlaTarget);
	g_aeopid[85].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloActlaTarget);

	g_aeopid[86].otyp = OTYP_Void;
	g_aeopid[86].grfeopid = 1;
	g_aeopid[86].optdat.ibSet = 0;
	g_aeopid[86].optdat.pvThunkFn = RefThunkAloRESUME_ACTLA;
	g_aeopid[86].optdat.ibGet = 0;

	g_aeopid[87].otyp = OTYP_Void;
	g_aeopid[87].grfeopid = 1;
	g_aeopid[87].optdat.ibSet = 0;
	g_aeopid[87].optdat.pvThunkFn = RefThunkAloPAUSE_ACTLA;
	g_aeopid[87].optdat.ibGet = 0;

	g_aeopid[88].otyp = OTYP_Void;
	g_aeopid[88].grfeopid = 1025;
	g_aeopid[88].optdat.ibSet = 3;
	g_aeopid[88].optdat.pvThunkFnUser = RefThunkAloROTATION_MATCHES_VELOCITY;
	g_aeopid[88].optdat.pvThunkFn = RefThunkAloROTATION_MATCHES_VELOCITY;

	g_aeopid[89].otyp = OTYP_List | OTYP_Lo;
	g_aeopid[89].grfeopid = 4;
	g_aeopid[89].optdat.pvThunkFnUser = (PVTHUNKFN)FGetAloChildrenList;

	g_aeopid[90].otyp = OTYP_Void;
	g_aeopid[90].grfeopid = 1;
	g_aeopid[90].optdat.ibGet = 0;
	g_aeopid[90].optdat.ibSet = 2;
	g_aeopid[90].optdat.pvThunkFn = RefThunkAloSCROLLING_MASTER_SPEEDS;

	g_aeopid[91].otyp = OTYP_Icek;
	g_aeopid[91].grfeopid = 288;
	g_aeopid[91].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloTargetAttacks);
	g_aeopid[91].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloTargetAttacks);

	g_aeopid[92].otyp = OTYP_Float;
	g_aeopid[92].grfeopid = 288;
	g_aeopid[92].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloTargetRadius);
	g_aeopid[92].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloTargetRadius);

	g_aeopid[93].otyp = OTYP_Bool;
	g_aeopid[93].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloTargetHitTest);
	g_aeopid[93].grfeopid = 288;
	g_aeopid[93].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloTargetHitTest);

	g_aeopid[94].otyp = OTYP_Void;
	g_aeopid[94].grfeopid = 1;
	g_aeopid[94].optdat.ibSet = 1;
	g_aeopid[94].optdat.ibGet = 0;
	g_aeopid[94].optdat.pvThunkFn = RefThunkAloEYES_CLOSED;

	g_aeopid[95].grfeopid = 288;
	g_aeopid[95].otyp = OTYP_Int;
	g_aeopid[95].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloCelRgba);
	g_aeopid[95].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloCelRgba);

	g_aeopid[96].otyp = OTYP_Int;
	g_aeopid[96].grfeopid = 32;
	g_aeopid[96].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloOverrideCel);

	g_aeopid[97].otyp = OTYP_Exrk;
	g_aeopid[97].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloThrobKind);
	g_aeopid[97].grfeopid = 292;
	g_aeopid[97].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloThrobKind);
	g_aeopid[97].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloThrobKind;

	g_aeopid[98].otyp = OTYP_Vector;
	g_aeopid[98].grfeopid = 292;
	g_aeopid[98].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloThrobInColor;
	g_aeopid[98].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloThrobInColor);
	g_aeopid[98].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloThrobInColor);

	g_aeopid[99].otyp = OTYP_Vector;
	g_aeopid[99].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloThrobOutColor;
	g_aeopid[99].grfeopid = 292;
	g_aeopid[99].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloThrobOutColor);
	g_aeopid[99].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloThrobOutColor);

	g_aeopid[100].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloThrobDtInOut;
	g_aeopid[100].grfeopid = 292;
	g_aeopid[100].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloThrobDtInOut);
	g_aeopid[100].otyp = OTYP_Float;
	g_aeopid[100].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloThrobDtInOut);

	g_aeopid[101].otyp = OTYP_Sfxid;
	g_aeopid[101].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloSfxid;
	g_aeopid[101].grfeopid = 292;
	g_aeopid[101].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloSfxid);
	g_aeopid[101].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloSfxidSpl);

	g_aeopid[102].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloSStart;
	g_aeopid[102].otyp = OTYP_Float;
	g_aeopid[102].grfeopid = 260;
	g_aeopid[102].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloSStart);

	g_aeopid[103].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloSFull;
	g_aeopid[103].grfeopid = 260;
	g_aeopid[103].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloSFull);
	g_aeopid[103].otyp = OTYP_Float;

	g_aeopid[104].otyp = OTYP_Float;
	g_aeopid[104].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloUVolume;
	g_aeopid[104].grfeopid = 292;
	g_aeopid[104].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloUVolume);
	g_aeopid[104].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloUVolumeSpl);

	g_aeopid[105].otyp = OTYP_Float;
	g_aeopid[105].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloUPitch;
	g_aeopid[105].grfeopid = 292;
	g_aeopid[105].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloUPitchSpl);
	g_aeopid[105].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloUPitch);

	g_aeopid[106].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloSndRepeat;
	g_aeopid[106].otyp = OTYP_Lm;
	g_aeopid[106].grfeopid = 260;
	g_aeopid[106].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloSndRepeat);

	g_aeopid[107].otyp = OTYP_Void;
	g_aeopid[107].optdat.ibSet = 5;
	g_aeopid[107].grfeopid = 1;
	g_aeopid[107].optdat.ibGet = 0;
	g_aeopid[107].optdat.pvThunkFn = RefThunkAloSTART_SOUND;

	g_aeopid[108].otyp = OTYP_Void;
	g_aeopid[108].optdat.pvThunkFn = RefThunkAloSTOP_SOUND;
	g_aeopid[108].grfeopid = 1;
	g_aeopid[108].optdat.ibGet = 0;
	g_aeopid[108].optdat.ibSet = 0;

	g_aeopid[109].grfeopid = 260;
	g_aeopid[109].otyp = OTYP_Float;
	g_aeopid[109].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloUDoppler;
	g_aeopid[109].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloUDoppler);

	g_aeopid[110].otyp = OTYP_Cfk;
	g_aeopid[110].grfeopid = 292;
	g_aeopid[110].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloInteractCane;
	g_aeopid[110].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloInteractCane);
	g_aeopid[110].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloInteractCane);

	g_aeopid[111].otyp = OTYP_Cfk;
	g_aeopid[111].grfeopid = 292;
	g_aeopid[111].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloInteractCaneSweep;
	g_aeopid[111].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloInteractCaneSweep);
	g_aeopid[111].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloInteractCaneSweep);

	g_aeopid[112].otyp = OTYP_Cfk;
	g_aeopid[112].grfeopid = 292;
	g_aeopid[112].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloInteractCaneRush;
	g_aeopid[112].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloInteractCaneRush);
	g_aeopid[112].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloInteractCaneRush);

	g_aeopid[113].otyp = OTYP_Cfk;
	g_aeopid[113].grfeopid = 292;
	g_aeopid[113].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloInteractCaneSmash);
	g_aeopid[113].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloInteractCaneSmash;
	g_aeopid[113].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloInteractCaneSmash);

	g_aeopid[114].otyp = OTYP_Cfk;
	g_aeopid[114].grfeopid = 292;
	g_aeopid[114].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloInteractBomb);
	g_aeopid[114].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloInteractBomb;
	g_aeopid[114].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloInteractBomb);

	g_aeopid[115].grfeopid = 292;
	g_aeopid[115].otyp = OTYP_Cfk;
	g_aeopid[115].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloInteractShock;
	g_aeopid[115].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloInteractShock);
	g_aeopid[115].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloInteractShock);

	g_aeopid[116].otyp = OTYP_Oid;
	g_aeopid[116].grfeopid = 32;
	g_aeopid[116].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloPoseCombo);

	g_aeopid[117].otyp = OTYP_Void;
	g_aeopid[117].optdat.pvThunkFn = RefThunkAloFADE_IN;
	g_aeopid[117].grfeopid = 1;
	g_aeopid[117].optdat.ibSet = 1;
	g_aeopid[117].optdat.ibGet = 0;

	g_aeopid[118].otyp = OTYP_Void;
	g_aeopid[118].optdat.pvThunkFn = RefThunkAloFADE_OUT;
	g_aeopid[118].grfeopid = 1;
	g_aeopid[118].optdat.ibSet = 1;
	g_aeopid[118].optdat.ibGet = 0;

	g_aeopid[119].grfeopid = 258;
	g_aeopid[119].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALO, fRealClock));
	g_aeopid[119].otyp = OTYP_Bool;
	g_aeopid[119].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloRealClock);

	g_aeopid[120].otyp = OTYP_Map;
	g_aeopid[120].grfeopid = 1;
	g_aeopid[120].optdat.ibSet = 1;
	g_aeopid[120].optdat.pvThunkFn = RefThunkAloCURRENT_SMA;
	g_aeopid[120].optdat.ibGet = 0;

	g_aeopid[121].otyp = OTYP_Asegbl;
	g_aeopid[121].grfeopid = 1;
	g_aeopid[121].optdat.ibSet = 1;
	g_aeopid[121].optdat.pvThunkFn = RefThunkAloCURRENT_ASEGA;
	g_aeopid[121].optdat.ibGet = 0;

	g_aeopid[122].grfeopid = 1;
	g_aeopid[122].otyp = OTYP_Asegbl;
	g_aeopid[122].optdat.ibSet = 0;
	g_aeopid[122].optdat.pvThunkFn = RefThunkAloNEAREST_ASEGA;
	g_aeopid[122].optdat.ibGet = 0;

	g_aeopid[123].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloForceCameraFade);
	g_aeopid[123].otyp = OTYP_Bool;
	g_aeopid[123].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloForceCameraFade);
	g_aeopid[123].grfeopid = 288;

	g_aeopid[124].otyp = OTYP_Void;
	g_aeopid[124].optdat.ibSet = 1;
	g_aeopid[124].grfeopid = 1;
	g_aeopid[124].optdat.ibGet = 0;
	g_aeopid[124].optdat.pvThunkFn = RefThunkAloMATCH_OTHER_OBJECT;

	g_aeopid[125].otyp = OTYP_Bool;
	g_aeopid[125].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetAloHidden);
	g_aeopid[125].optdat.pvThunkFnUser = (PVTHUNKFN)GetAloHidden;
	g_aeopid[125].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetAloHidden);
	g_aeopid[125].grfeopid = 292;

	g_aeopid[126].otyp = OTYP_Void;
	g_aeopid[126].optdat.ibSet = 2;
	g_aeopid[126].grfeopid = 1;
	g_aeopid[126].optdat.pvThunkFn = RefThunkAloSET_TRANSFORM_BASIS;
	g_aeopid[126].optdat.ibGet = 0;


	g_aeopid[127].otyp = OTYP_Float;
	g_aeopid[127].grfeopid = 34;
	g_aeopid[127].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSoMass);
	g_aeopid[127].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SO, m));

	g_aeopid[128].otyp = OTYP_Rumk;
	g_aeopid[128].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSoMtlk);
	g_aeopid[128].grfeopid = 288;
	g_aeopid[128].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSoMtlk);

	g_aeopid[129].otyp = OTYP_Vector;
	g_aeopid[129].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SO, dvGravity));
	g_aeopid[129].grfeopid = 146;
	g_aeopid[129].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SO, dvGravity));
	g_aeopid[129].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SO, dvGravity));

	g_aeopid[130].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSoNoInteract);
	g_aeopid[130].grfeopid = 256;
	g_aeopid[130].otyp = OTYP_Bool;

	g_aeopid[131].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSoCnstrForce);
	g_aeopid[131].otyp = OTYP_Cnstr;
	g_aeopid[131].grfeopid = 288;
	g_aeopid[131].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSoCnstrForce);

	g_aeopid[132].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSoCnstrTorque);
	g_aeopid[132].otyp = OTYP_Cnstr;
	g_aeopid[132].grfeopid = 288;
	g_aeopid[132].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSoCnstrTorque);

	g_aeopid[133].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSoSphere);
	g_aeopid[133].otyp = OTYP_Float;
	g_aeopid[133].grfeopid = 256;

	g_aeopid[134].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SO, gBuoyancy));
	g_aeopid[134].otyp = OTYP_Float;
	g_aeopid[134].grfeopid = 2194;
	g_aeopid[134].optdat.fDef = 1056964608;
	g_aeopid[134].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SO, gBuoyancy));
	g_aeopid[134].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SO, gBuoyancy));

	g_aeopid[135].otyp = OTYP_Float;
	g_aeopid[135].grfeopid = 2194;
	g_aeopid[135].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SO, gViscosity));
	g_aeopid[135].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SO, gViscosity));
	g_aeopid[135].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SO, gViscosity));
	g_aeopid[135].optdat.fDef = 1065353216;

	g_aeopid[136].grfeopid = 288;
	g_aeopid[136].otyp = OTYP_Bool;
	g_aeopid[136].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSoIgnoreLocked);
	g_aeopid[136].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSoIgnoreLocked);

	g_aeopid[137].otyp = OTYP_Bool;
	g_aeopid[137].grfeopid = 256;
	g_aeopid[137].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSoIceable);

	g_aeopid[138].otyp = OTYP_Bool;
	g_aeopid[138].grfeopid = 288;
	g_aeopid[138].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSoNoGravity);
	g_aeopid[138].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSoNoGravity);

	g_aeopid[139].otyp = OTYP_Bool;
	g_aeopid[139].grfeopid = 288;
	g_aeopid[139].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSoNoXpsAll);
	g_aeopid[139].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSoNoXpsAll);

	g_aeopid[140].otyp = OTYP_Bool;
	g_aeopid[140].grfeopid = 288;
	g_aeopid[140].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSoNoXpsSelf);
	g_aeopid[140].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSoNoXpsSelf);

	g_aeopid[141].grfeopid = 288;
	g_aeopid[141].otyp = OTYP_Bool;
	g_aeopid[141].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSoNoXpsCenter);
	g_aeopid[141].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSoNoXpsCenter);

	g_aeopid[142].optdat.pvThunkFnUser = (PVTHUNKFNUSER)FGetSoContactList;
	g_aeopid[142].otyp = OTYP_List | OTYP_So;
	g_aeopid[142].grfeopid = 4;

	g_aeopid[143].otyp = OTYP_Boidk;
	g_aeopid[143].grfeopid = 146;
	g_aeopid[143].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SO, zok));
	g_aeopid[143].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SO, zok));
	g_aeopid[143].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SO, zok));

	g_aeopid[144].otyp = OTYP_Float;
	g_aeopid[144].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SO, sRadiusSelf));
	g_aeopid[144].grfeopid = 2;

	g_aeopid[145].otyp = OTYP_Sgs;
	g_aeopid[145].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSoCameraStyle);
	g_aeopid[145].grfeopid = 256;

	g_aeopid[146].optdat.pvThunkFnUser = RefThunkSoEDGE_GRAB;
	g_aeopid[146].otyp = OTYP_Void;
	g_aeopid[146].grfeopid = 1025;
	g_aeopid[146].optdat.ibSet = 1;
	g_aeopid[146].optdat.pvThunkFn = RefThunkSoEDGE_GRAB;

	g_aeopid[147].otyp = OTYP_Bool;
	g_aeopid[147].grfeopid = 1;
	g_aeopid[147].optdat.ibGet = 0;
	g_aeopid[147].optdat.ibSet = 1;
	g_aeopid[147].optdat.pvThunkFn = RefThunkBspPOINT_IN_BSP_QUICK;

	g_aeopid[148].optdat.pvThunkFnUser = (PVTHUNKFN)GetSoCenterXp;
	g_aeopid[148].grfeopid = 292;
	g_aeopid[148].otyp = OTYP_Bool;
	g_aeopid[148].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSoCenterXp);
	g_aeopid[148].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSoCenterXp);

	g_aeopid[149].otyp = OTYP_Vector;
	g_aeopid[149].optdat.ibGet = static_cast<std::intptr_t>(offsetof(XFM, posLocal));
	g_aeopid[149].grfeopid = 18;
	g_aeopid[149].optdat.ibSet = static_cast<std::intptr_t>(offsetof(XFM, posLocal));

	g_aeopid[150].otyp = OTYP_Matrix;
	g_aeopid[150].optdat.ibGet = static_cast<std::intptr_t>(offsetof(XFM, matLocal));
	g_aeopid[150].grfeopid = 18;
	g_aeopid[150].optdat.ibSet = static_cast<std::intptr_t>(offsetof(XFM, matLocal));

	g_aeopid[151].optdat.pvThunkFnUser = (PVTHUNKFN)GetXfmPos;
	g_aeopid[151].otyp = OTYP_Vector;
	g_aeopid[151].grfeopid = 4;

	g_aeopid[152].otyp = OTYP_Matrix;
	g_aeopid[152].grfeopid = 4;
	g_aeopid[152].optdat.pvThunkFnUser = (PVTHUNKFN)GetXfmMat;

	g_aeopid[153].grfeopid = 18;
	g_aeopid[153].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WARP, v));
	g_aeopid[153].otyp = OTYP_Vector;
	g_aeopid[153].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WARP, v));

	g_aeopid[154].otyp = OTYP_Float;
	g_aeopid[154].grfeopid = 146;
	g_aeopid[154].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WARP, v) + offsetof(glm::vec3, x));
	g_aeopid[154].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WARP, v) + offsetof(glm::vec3, x));
	g_aeopid[154].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WARP, v) + offsetof(glm::vec3, x));

	g_aeopid[155].grfeopid = 146;
	g_aeopid[155].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WARP, v) + offsetof(glm::vec3, y));
	g_aeopid[155].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WARP, v) + offsetof(glm::vec3, y));
	g_aeopid[155].otyp = OTYP_Float;
	g_aeopid[155].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WARP, v) + offsetof(glm::vec3, y));

	g_aeopid[156].grfeopid = 146;
	g_aeopid[156].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WARP, v) + offsetof(glm::vec3, z));
	g_aeopid[156].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WARP, v) + offsetof(glm::vec3, z));
	g_aeopid[156].otyp = OTYP_Float;
	g_aeopid[156].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WARP, v) + offsetof(glm::vec3, z));

	g_aeopid[157].grfeopid = 130;
	g_aeopid[157].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WARP, fDefault));
	g_aeopid[157].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WARP, fDefault));
	g_aeopid[157].otyp = OTYP_Bool;

	g_aeopid[158].grfeopid = 130;
	g_aeopid[158].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WARP, fDefaultSpecial));
	g_aeopid[158].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WARP, fDefaultSpecial));
	g_aeopid[158].otyp = OTYP_Bool;

	g_aeopid[159].grfeopid = 146;
	g_aeopid[159].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WARP, oidAlias));
	g_aeopid[159].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WARP, oidAlias));
	g_aeopid[159].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WARP, oidAlias));
	g_aeopid[159].otyp = OTYP_Oid;

	g_aeopid[160].otyp = OTYP_Float;
	g_aeopid[160].grfeopid = 290;
	g_aeopid[160].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetWarpCmInitAngle);
	g_aeopid[160].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WARP, radCmInit));
	g_aeopid[160].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetWarpCmInitAngle);

	g_aeopid[161].grfeopid = 1025;
	g_aeopid[161].optdat.pvThunkFnUser = RefThunkWarpSET_SM_GOAL;
	g_aeopid[161].otyp = OTYP_Void;
	g_aeopid[161].optdat.ibSet = 4;
	g_aeopid[161].optdat.pvThunkFn = RefThunkWarpSET_SM_GOAL;

	g_aeopid[162].otyp = OTYP_Void;
	g_aeopid[162].grfeopid = 1;
	g_aeopid[162].optdat.ibGet = 0;
	g_aeopid[162].optdat.ibSet = 0;
	g_aeopid[162].optdat.pvThunkFn = RefThunkWarpTRIGGER;

	g_aeopid[163].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXIT, oidWarp));
	g_aeopid[163].otyp = OTYP_Oid;
	g_aeopid[163].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXIT, oidWarp));
	g_aeopid[163].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EXIT, oidWarp));
	g_aeopid[163].optdat.fDef = -1;
	g_aeopid[163].grfeopid = 2194;

	g_aeopid[164].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXIT, edkAlt));
	g_aeopid[164].otyp = OTYP_Egk;
	g_aeopid[164].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXIT, edkAlt));
	g_aeopid[164].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EXIT, edkAlt));
	g_aeopid[164].optdat.fDef = -1;
	g_aeopid[164].grfeopid = 2194;

	g_aeopid[165].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXIT, oidWarpAlt));
	g_aeopid[165].otyp = OTYP_Oid;
	g_aeopid[165].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXIT, oidWarpAlt));
	g_aeopid[165].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EXIT, oidWarpAlt));
	g_aeopid[165].optdat.fDef = -1;
	g_aeopid[165].grfeopid = 2194;

	g_aeopid[166].otyp = OTYP_Wipek;
	g_aeopid[166].grfeopid = 2194;
	g_aeopid[166].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXIT, wipek));
	g_aeopid[166].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXIT, wipek));
	g_aeopid[166].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EXIT, wipek));
	g_aeopid[166].optdat.fDef = 0;

	g_aeopid[167].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXIT, dtUnblock));
	g_aeopid[167].otyp = OTYP_Float;
	g_aeopid[167].grfeopid = 2194;
	g_aeopid[167].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXIT, dtUnblock));
	g_aeopid[167].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EXIT, dtUnblock));
	g_aeopid[167].optdat.fDef = 0;

	g_aeopid[168].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXIT, fDefault));
	g_aeopid[168].grfeopid = 2194;
	g_aeopid[168].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXIT, fDefault));
	g_aeopid[168].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EXIT, fDefault));
	g_aeopid[168].otyp = OTYP_Bool;
	g_aeopid[168].optdat.fDef = 0;

	g_aeopid[169].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXIT, fKeyed));
	g_aeopid[169].grfeopid = 2194;
	g_aeopid[169].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXIT, fKeyed));
	g_aeopid[169].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EXIT, fKeyed));
	g_aeopid[169].otyp = OTYP_Bool;
	g_aeopid[169].optdat.fDef = 0;

	g_aeopid[170].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXIT, fFollowDefault));
	g_aeopid[170].grfeopid = 2194;
	g_aeopid[170].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXIT, fFollowDefault));
	g_aeopid[170].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EXIT, fFollowDefault));
	g_aeopid[170].otyp = OTYP_Bool;
	g_aeopid[170].optdat.fDef = 0;

	g_aeopid[171].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXIT, fUseDefaultExit));
	g_aeopid[171].grfeopid = 2194;
	g_aeopid[171].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXIT, fUseDefaultExit));
	g_aeopid[171].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EXIT, fUseDefaultExit));
	g_aeopid[171].otyp = OTYP_Bool;
	g_aeopid[171].optdat.fDef = 0;

	g_aeopid[172].grfeopid = 2194;
	g_aeopid[172].otyp = OTYP_Bool;
	g_aeopid[172].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXIT, fTotals));
	g_aeopid[172].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EXIT, fTotals));
	g_aeopid[172].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXIT, fTotals));
	g_aeopid[172].optdat.fDef = 0;

	g_aeopid[173].otyp = OTYP_Float;
	g_aeopid[173].grfeopid = 146;
	g_aeopid[173].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXIT, dtTriggerWipe));
	g_aeopid[173].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EXIT, dtTriggerWipe));
	g_aeopid[173].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXIT, dtTriggerWipe));

	g_aeopid[174].otyp = OTYP_Void;
	g_aeopid[174].grfeopid = 1;
	g_aeopid[174].optdat.ibGet = 0;
	g_aeopid[174].optdat.ibSet = 0;
	g_aeopid[174].optdat.pvThunkFn = RefThunkExitTRIGGER;

	g_aeopid[175].otyp = 4200;
	g_aeopid[175].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXIT, exits));
	g_aeopid[175].grfeopid = 34;
	g_aeopid[175].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetExitExits);

	g_aeopid[176].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PNT, posLocal));
	g_aeopid[176].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PNT, posLocal));
	g_aeopid[176].grfeopid = 18;
	g_aeopid[176].otyp = OTYP_Vector;

	g_aeopid[177].otyp = OTYP_Vector;
	g_aeopid[177].grfeopid = 4;
	g_aeopid[177].optdat.pvThunkFnUser = (PVTHUNKFN)GetPntPos;

	g_aeopid[178].otyp = OTYP_Segrpt;
	g_aeopid[178].grfeopid = 2176;
	g_aeopid[178].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ASEG, segrpt));
	g_aeopid[178].optdat.fDef = 0;

	g_aeopid[179].grfeopid = 2176;
	g_aeopid[179].otyp = OTYP_Bool;
	g_aeopid[179].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ASEG, fDefault));
	g_aeopid[179].optdat.fDef = 0;

	g_aeopid[180].grfeopid = 2176;
	g_aeopid[180].otyp = OTYP_Bool;
	g_aeopid[180].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ASEG, fHandsOff));
	g_aeopid[180].optdat.fDef = 0;

	g_aeopid[181].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ASEG, nPriority));
	g_aeopid[181].grfeopid = 128;
	g_aeopid[181].otyp = OTYP_Int;

	g_aeopid[182].grfeopid = 2194;
	g_aeopid[182].otyp = OTYP_Float;
	g_aeopid[182].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ASEG, svtMaster));
	g_aeopid[182].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ASEG, svtMaster));
	g_aeopid[182].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ASEG, svtMaster));
	g_aeopid[182].optdat.fDef = 1065353216;

	g_aeopid[183].grfeopid = 128;
	g_aeopid[183].otyp = OTYP_Clq;
	g_aeopid[183].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ASEG, clqMasterSuck));

	g_aeopid[184].grfeopid = 128;
	g_aeopid[184].otyp = OTYP_Lm;
	g_aeopid[184].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ASEG, lmMasterSuck));

	g_aeopid[185].grfeopid = 1;
	g_aeopid[185].otyp = OTYP_Asegbl;
	g_aeopid[185].optdat.ibGet = 0;
	g_aeopid[185].optdat.ibSet = 4;
	g_aeopid[185].optdat.pvThunkFn = RefThunkAsegAPPLY;

	g_aeopid[186].grfeopid = 1;
	g_aeopid[186].otyp = OTYP_Asegbl;
	g_aeopid[186].optdat.ibSet = 5;
	g_aeopid[186].optdat.ibGet = 0;
	g_aeopid[186].optdat.pvThunkFn = RefThunkAsegENSURE;

	g_aeopid[187].grfeopid = 1;
	g_aeopid[187].otyp = OTYP_Float;
	g_aeopid[187].optdat.ibGet = 0;
	g_aeopid[187].optdat.pvThunkFn = RefThunkAsegFIND_LABEL;
	g_aeopid[187].optdat.ibSet = 1;

	g_aeopid[188].otyp = OTYP_Float;
	g_aeopid[188].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ASEG, tMax));
	g_aeopid[188].grfeopid = 2;

	g_aeopid[189].grfeopid = 130;
	g_aeopid[189].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ASEG, fRealClock));
	g_aeopid[189].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ASEG, fRealClock));
	g_aeopid[189].otyp = OTYP_Bool;

	g_aeopid[190].grfeopid = 2;
	g_aeopid[190].optdat.ibGet = static_cast<int>(offsetof(ASEGA, paseg));
	g_aeopid[190].otyp = OTYP_Explo;

	g_aeopid[191].grfeopid = 2;
	g_aeopid[191].optdat.ibGet = static_cast<int>(offsetof(ASEGA, paloRoot));
	g_aeopid[191].otyp = OTYP_Alo;

	g_aeopid[192].grfeopid = 2;
	g_aeopid[192].optdat.ibGet = static_cast<int>(offsetof(ASEGA, tLocal));
	g_aeopid[192].otyp = OTYP_Float;

	g_aeopid[193].grfeopid = 34;
	g_aeopid[193].optdat.ibGet = static_cast<std::ptrdiff_t>(offsetof(ASEGA, svtLocal));
	g_aeopid[193].optdat.pfnset = reinterpret_cast<PFNRAW>(SetAsegaSpeed);
	g_aeopid[193].otyp = OTYP_Float;

	g_aeopid[194].grfeopid = 34;
	g_aeopid[194].optdat.ibGet = static_cast<std::ptrdiff_t>(offsetof(ASEGA, svtMaster));
	g_aeopid[194].optdat.pfnset = reinterpret_cast<PFNRAW>(SetAsegaMasterSpeed);
	g_aeopid[194].otyp = OTYP_Float;

	g_aeopid[195].grfeopid = 34;
	g_aeopid[195].optdat.ibGet = static_cast<int>(offsetof(ASEGA, nPriority));
	g_aeopid[195].optdat.pfnset = reinterpret_cast<PFNRAW>(SetAsegaPriority);
	g_aeopid[195].otyp = OTYP_Int;

	g_aeopid[196].grfeopid = 34;
	g_aeopid[196].optdat.ibGet = static_cast<int>(offsetof(ASEGA, fHandsOff));
	g_aeopid[196].optdat.pfnset = reinterpret_cast<PFNRAW>(SetAsegaHandsOff);
	g_aeopid[196].otyp = OTYP_Bool;

	g_aeopid[197].otyp = OTYP_Void;
	g_aeopid[197].grfeopid = 1;
	g_aeopid[197].optdat.ibSet = 3;
	g_aeopid[197].optdat.ibGet = 0;
	g_aeopid[197].optdat.pvThunkFn = RefThunkAsegaSEEK;

	g_aeopid[198].optdat.pvThunkFn = RefThunkAsegaRETRACT;
	g_aeopid[198].otyp = OTYP_Void;
	g_aeopid[198].grfeopid = 1;
	g_aeopid[198].optdat.ibSet = 0;
	g_aeopid[198].optdat.ibGet = 0;

	g_aeopid[199].otyp = OTYP_Void;
	g_aeopid[199].grfeopid = 1;
	g_aeopid[199].optdat.ibSet = 1;
	g_aeopid[199].optdat.ibGet = 0;
	g_aeopid[199].optdat.pvThunkFn = RefThunkAsegaSNAP;

	g_aeopid[200].otyp = OTYP_Void;
	g_aeopid[200].grfeopid = 1;
	g_aeopid[200].optdat.ibSet = 2;
	g_aeopid[200].optdat.ibGet = 0;
	g_aeopid[200].optdat.pvThunkFn = RefThunkAsegaRETRACT_ACTSEG;

	g_aeopid[201].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WATER, fSplash));
	g_aeopid[201].grfeopid = 2194;
	g_aeopid[201].otyp = OTYP_Bool;
	g_aeopid[201].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WATER, fSplash));
	g_aeopid[201].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WATER, fSplash));
	g_aeopid[201].optdat.fDef = 1;

	g_aeopid[202].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WATER, fZap));
	g_aeopid[202].grfeopid = 2194;
	g_aeopid[202].otyp = OTYP_Bool;
	g_aeopid[202].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WATER, fZap));
	g_aeopid[202].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WATER, fZap));
	g_aeopid[202].optdat.fDef = 1;

	g_aeopid[203].otyp = OTYP_Vector;
	g_aeopid[203].grfeopid = 146;
	g_aeopid[203].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WATER, vCurrent));
	g_aeopid[203].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WATER, vCurrent));
	g_aeopid[203].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WATER, vCurrent));

	g_aeopid[204].grfeopid = 4352;
	g_aeopid[204].otyp = OTYP_Oid;
	g_aeopid[204].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddZpdZapObject);
	g_aeopid[204].optdat.pfnensure = (PFNENSURE)PzpdEnsureWater;

	g_aeopid[205].optdat.ibSet = reinterpret_cast<std::intptr_t>(&AddZpdZapLo);
	g_aeopid[205].otyp = OTYP_Lo;
	g_aeopid[205].grfeopid = 4128;
	g_aeopid[205].optdat.pfnensure = (PFNENSURE)PzpdEnsureWater;

	g_aeopid[206].otyp = OTYP_Lo;
	g_aeopid[206].grfeopid = 4128;
	g_aeopid[206].optdat.ibSet = reinterpret_cast<std::intptr_t>(&RemoveZpdZapLo);
	g_aeopid[206].optdat.pfnensure = (PFNENSURE)PzpdEnsureWater;

	g_aeopid[207].otyp = OTYP_Void;
	g_aeopid[207].grfeopid = 1;
	g_aeopid[207].optdat.ibGet = 0;
	g_aeopid[207].optdat.ibSet = 0;
	g_aeopid[207].optdat.pvThunkFn = RefThunkWaterRESET_THROW_COUNT;

	g_aeopid[208].otyp = OTYP_Float;
	g_aeopid[208].grfeopid = 146;
	g_aeopid[208].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WATER, zpd) + offsetof(ZPD, dzThrow));
	g_aeopid[208].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WATER, zpd) + offsetof(ZPD, dzThrow));
	g_aeopid[208].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WATER, zpd) + offsetof(ZPD, dzThrow));

	g_aeopid[209].grfeopid = 146;
	g_aeopid[209].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WATER, zpd) + offsetof(ZPD, fThrowCut));
	g_aeopid[209].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WATER, zpd) + offsetof(ZPD, fThrowCut));
	g_aeopid[209].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WATER, zpd) + offsetof(ZPD, fThrowCut));
	g_aeopid[209].otyp = OTYP_Bool;

	g_aeopid[210].grfeopid = 146;
	g_aeopid[210].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WATER, fWaterWakeSound));
	g_aeopid[210].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WATER, fWaterWakeSound));
	g_aeopid[210].otyp = OTYP_Bool;
	g_aeopid[210].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WATER, fWaterWakeSound));

	g_aeopid[211].otyp = OTYP_Bool;
	g_aeopid[211].grfeopid = 146;
	g_aeopid[211].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WATER, fSplashDroplets));
	g_aeopid[211].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WATER, fSplashDroplets));
	g_aeopid[211].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WATER, fSplashDroplets));

	g_aeopid[212].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetBrkExclude);
	g_aeopid[212].otyp = OTYP_Oid;
	g_aeopid[212].grfeopid = 256;

	g_aeopid[213].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetBrkRemain);
	g_aeopid[213].otyp = OTYP_Oid;
	g_aeopid[213].grfeopid = 256;

	g_aeopid[214].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetBrkFixed);
	g_aeopid[214].otyp = OTYP_Oid;
	g_aeopid[214].grfeopid = 256;

	g_aeopid[215].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetBrkRemainFixed);
	g_aeopid[215].otyp = OTYP_Oid;
	g_aeopid[215].grfeopid = 256;

	g_aeopid[216].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, fRemain));
	g_aeopid[216].otyp = OTYP_Bool;
	g_aeopid[216].grfeopid = 2194;
	g_aeopid[216].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRK, fRemain));
	g_aeopid[216].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRK, fRemain));
	g_aeopid[216].optdat.fDef = 0;

	g_aeopid[217].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, rvAbsorbWhack));
	g_aeopid[217].otyp = OTYP_Float;
	g_aeopid[217].grfeopid = 146;
	g_aeopid[217].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRK, rvAbsorbWhack));
	g_aeopid[217].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRK, rvAbsorbWhack));

	g_aeopid[218].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, oidHiPri));
	g_aeopid[218].optdat.fDef = -1;
	g_aeopid[218].otyp = OTYP_Oid;
	g_aeopid[218].grfeopid = 2176;

	g_aeopid[219].otyp = OTYP_Lm;
	g_aeopid[219].grfeopid = 146;
	g_aeopid[219].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, lmBreakPosToV));
	g_aeopid[219].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRK, lmBreakPosToV));
	g_aeopid[219].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRK, lmBreakPosToV));

	g_aeopid[220].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, svBreak));
	g_aeopid[220].optdat.fDef = 1133903872;
	g_aeopid[220].otyp = OTYP_Float;
	g_aeopid[220].grfeopid = 2194;
	g_aeopid[220].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRK, svBreak));
	g_aeopid[220].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRK, svBreak));

	g_aeopid[221].otyp = OTYP_Float;
	g_aeopid[221].grfeopid = 2194;
	g_aeopid[221].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, swBreak));
	g_aeopid[221].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRK, swBreak));
	g_aeopid[221].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRK, swBreak));
	g_aeopid[221].optdat.fDef = 1077936128;

	g_aeopid[222].otyp = OTYP_Oid;
	g_aeopid[222].grfeopid = 256;
	g_aeopid[222].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddBrkTouchObject);

	g_aeopid[223].otyp = OTYP_Cid;
	g_aeopid[223].grfeopid = 256;
	g_aeopid[223].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddBrkTouchClass);

	g_aeopid[224].otyp = OTYP_Cid;
	g_aeopid[224].grfeopid = 256;
	g_aeopid[224].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddBrkAbsorbClass);

	g_aeopid[225].optdat.pvThunkFnUser = (PVTHUNKFN)GetBrkOnPlayer;
	g_aeopid[225].grfeopid = 292;
	g_aeopid[225].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetBrkOnPlayer);
	g_aeopid[225].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetBrkOnPlayer);
	g_aeopid[225].otyp = OTYP_Bool;

	g_aeopid[226].grfeopid = 292;
	g_aeopid[226].optdat.pvThunkFnUser = (PVTHUNKFN)GetBrkOnBomb;
	g_aeopid[226].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetBrkOnBomb);
	g_aeopid[226].otyp = OTYP_Bool;
	g_aeopid[226].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetBrkOnBomb);

	g_aeopid[227].otyp = OTYP_Int;
	g_aeopid[227].grfeopid = 146;
	g_aeopid[227].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRK, cHitsToBreak));
	g_aeopid[227].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRK, cHitsToBreak));
	g_aeopid[227].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, cHitsToBreak));

	g_aeopid[228].optdat.pvThunkFn = RefThunkBrkBREAK;
	g_aeopid[228].otyp = OTYP_Void;
	g_aeopid[228].grfeopid = 1;
	g_aeopid[228].optdat.ibGet = 0;
	g_aeopid[228].optdat.ibSet = 0;

	g_aeopid[229].grfeopid = 34;
	g_aeopid[229].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRK, fBroken));
	g_aeopid[229].otyp = OTYP_Bool;
	g_aeopid[229].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetBrkBroken);

	g_aeopid[230].grfeopid = 146;
	g_aeopid[230].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRK, fIgnoreBrkp));
	g_aeopid[230].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, fIgnoreBrkp));
	g_aeopid[230].otyp = OTYP_Bool;
	g_aeopid[230].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRK, fIgnoreBrkp));

	g_aeopid[231].otyp = OTYP_Fhp;
	g_aeopid[231].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, exrk));
	g_aeopid[231].grfeopid = 2194;
	g_aeopid[231].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRK, exrk));
	g_aeopid[231].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRK, exrk));
	g_aeopid[231].optdat.fDef = 1;

	g_aeopid[232].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, rExplodeScale));
	g_aeopid[232].otyp = OTYP_Float;
	g_aeopid[232].grfeopid = 146;
	g_aeopid[232].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRK, rExplodeScale));
	g_aeopid[232].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRK, rExplodeScale));

	g_aeopid[233].otyp = OTYP_Int;
	g_aeopid[233].grfeopid = 146;
	g_aeopid[233].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRK, ccoin));
	g_aeopid[233].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRK, ccoin));
	g_aeopid[233].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, ccoin));

	g_aeopid[234].otyp = OTYP_Vector;
	g_aeopid[234].grfeopid = 146;
	g_aeopid[234].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRK, dposCoin));
	g_aeopid[234].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, dposCoin));
	g_aeopid[234].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRK, dposCoin));

	g_aeopid[235].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, fSuppressDifficultyUpdate));
	g_aeopid[235].grfeopid = 146;
	g_aeopid[235].otyp = OTYP_Bool;
	g_aeopid[235].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRK, fSuppressDifficultyUpdate));
	g_aeopid[235].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRK, fSuppressDifficultyUpdate));

	g_aeopid[236].otyp = OTYP_Sfxid;
	g_aeopid[236].grfeopid = 4242;
	g_aeopid[236].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SFX, sfxid));
	g_aeopid[236].optdat.pfnensure = (PFNENSURE)PsfxEnsureBrk;
	g_aeopid[236].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SFX, sfxid));
	g_aeopid[236].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SFX, sfxid));

	g_aeopid[237].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SFX, sStart));
	g_aeopid[237].otyp = OTYP_Float;
	g_aeopid[237].grfeopid = 4242;
	g_aeopid[237].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SFX, sStart));
	g_aeopid[237].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SFX, sStart));
	g_aeopid[237].optdat.pfnensure = (PFNENSURE)PsfxEnsureBrk;

	g_aeopid[238].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SFX, sFull));
	g_aeopid[238].otyp = OTYP_Float;
	g_aeopid[238].grfeopid = 4242;
	g_aeopid[238].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SFX, sFull));
	g_aeopid[238].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SFX, sFull));
	g_aeopid[238].optdat.pfnensure = (PFNENSURE)PsfxEnsureBrk;

	g_aeopid[239].grfeopid = 4242;
	g_aeopid[239].otyp = OTYP_Float;
	g_aeopid[239].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SFX, uVol));
	g_aeopid[239].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SFX, uVol));
	g_aeopid[239].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SFX, uVol));
	g_aeopid[239].optdat.pfnensure = (PFNENSURE)PsfxEnsureBrk;

	g_aeopid[240].grfeopid = 128;
	g_aeopid[240].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, oidExplBreak));
	g_aeopid[240].otyp = OTYP_Oid;

	g_aeopid[241].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRK, fSkipBreakEffects));
	g_aeopid[241].otyp = OTYP_Bool;
	g_aeopid[241].grfeopid = 146;
	g_aeopid[241].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRK, fSkipBreakEffects));
	g_aeopid[241].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRK, fSkipBreakEffects));


	g_aeopid[242].grfeopid = 0x92;
	g_aeopid[242].otyp = OTYP_Float;
	g_aeopid[242].optdat.ibGet = static_cast<std::ptrdiff_t>(offsetof(FRAGILE, rImpactMass));
	g_aeopid[242].optdat.ibSet = static_cast<std::ptrdiff_t>(offsetof(FRAGILE, rImpactMass));
	g_aeopid[242].optdat.ibSetUser = static_cast<std::ptrdiff_t>(offsetof(FRAGILE, rImpactMass));

	g_aeopid[243].grfeopid = 0x92;
	g_aeopid[243].otyp = OTYP_Cnstr;
	g_aeopid[243].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FRAGILE, cnstrImpactForce));
	g_aeopid[243].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FRAGILE, cnstrImpactForce));
	g_aeopid[243].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FRAGILE, cnstrImpactForce));
	g_aeopid[243].optdat.pfnget = (PFNRAW)GetFragileCnstrImpactForce;
	g_aeopid[243].optdat.pfnset = (PFNRAW)SetFragileCnstrImpactForce;

	g_aeopid[244].grfeopid = 0x92;
	g_aeopid[244].otyp = OTYP_Cnstr;
	g_aeopid[244].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FRAGILE, cnstrImpactTorque));
	g_aeopid[244].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FRAGILE, cnstrImpactTorque));
	g_aeopid[244].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FRAGILE, cnstrImpactTorque));
	g_aeopid[244].optdat.pfnget = (PFNRAW)GetFragileCnstrImpactTorque;
	g_aeopid[244].optdat.pfnset = (PFNRAW)SetFragileCnstrImpactTorque;

	g_aeopid[245].grfeopid = 0x12;
	g_aeopid[245].otyp = OTYP_So;
	g_aeopid[245].optdat.ibGet = 0x6cc;
	g_aeopid[245].optdat.ibSet = 0x6cc;
	g_aeopid[245].optdat.pfnget = (PFNRAW)GetFragilePsoImpacting;
	g_aeopid[245].optdat.pfnset = reinterpret_cast<PFNRAW>(SetFragilePsoImpacting);

	g_aeopid[246].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ZAPBREAK, zpk));
	g_aeopid[246].grfeopid = 146;
	g_aeopid[246].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ZAPBREAK, zpk));
	g_aeopid[246].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ZAPBREAK, zpk));
	g_aeopid[246].otyp = OTYP_Fapl;

	g_aeopid[247].otyp = OTYP_Float;
	g_aeopid[247].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRKP, dtFade));
	g_aeopid[247].grfeopid = 2194;
	g_aeopid[247].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRKP, dtFade));
	g_aeopid[247].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRKP, dtFade));
	g_aeopid[247].optdat.fDef = 1069547520;

	g_aeopid[248].grfeopid = 2194;
	g_aeopid[248].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRKP, dtMaxLifetime));
	g_aeopid[248].otyp = OTYP_Float;
	g_aeopid[248].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRKP, dtMaxLifetime));
	g_aeopid[248].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRKP, dtMaxLifetime));
	g_aeopid[248].optdat.fDef = 1092616192;

	g_aeopid[249].grfeopid = 290;
	g_aeopid[249].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetBrkpBrkps);
	g_aeopid[249].otyp = OTYP_Emitrk;
	g_aeopid[249].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRKP, brkps));
	g_aeopid[249].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetBrkpBrkps);

	g_aeopid[250].grfeopid = 146;
	g_aeopid[250].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BRKP, fIgnoreBrkp));
	g_aeopid[250].otyp = OTYP_Bool;
	g_aeopid[250].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BRKP, fIgnoreBrkp));
	g_aeopid[250].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BRKP, fIgnoreBrkp));

	g_aeopid[251].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddButtonAseg);
	g_aeopid[251].grfeopid = 256;
	g_aeopid[251].otyp = OTYP_Oid;

	g_aeopid[252].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddButtonOffAseg);
	g_aeopid[252].grfeopid = 256;
	g_aeopid[252].otyp = OTYP_Oid;

	g_aeopid[253].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddButtonPushObject);
	g_aeopid[253].grfeopid = 256;
	g_aeopid[253].otyp = OTYP_Oid;

	g_aeopid[254].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddButtonNoPushObject);
	g_aeopid[254].grfeopid = 256;
	g_aeopid[254].otyp = OTYP_Oid;

	g_aeopid[255].otyp = OTYP_Cid;
	g_aeopid[255].grfeopid = 256;
	g_aeopid[255].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddButtonPushClass);

	g_aeopid[256].otyp = OTYP_Cid;
	g_aeopid[256].grfeopid = 256;
	g_aeopid[256].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddButtonNoPushClass);

	g_aeopid[257].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BUTTON, btn) + offsetof(BTN, dtRepush));
	g_aeopid[257].otyp = OTYP_Float;
	g_aeopid[257].grfeopid = 2194;
	g_aeopid[257].optdat.fDef = 1065353216;
	g_aeopid[257].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BUTTON, btn) + offsetof(BTN, dtRepush));
	g_aeopid[257].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BUTTON, btn) + offsetof(BTN, dtRepush));

	g_aeopid[258].otyp = OTYP_Btnt;
	g_aeopid[258].grfeopid = 2176;
	g_aeopid[258].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BUTTON, btnt));
	g_aeopid[258].optdat.fDef = 0;

	g_aeopid[259].otyp = OTYP_Float;
	g_aeopid[259].grfeopid = 2176;
	g_aeopid[259].optdat.fDef = 1065353216;
	g_aeopid[259].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BUTTON, btn) + offsetof(BTN, svtAnimation));

	g_aeopid[260].grfeopid = 2176;
	g_aeopid[260].otyp = OTYP_Bool;
	g_aeopid[260].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BUTTON, btn) + offsetof(BTN, fSilent));
	g_aeopid[260].optdat.fDef = 0;

	g_aeopid[261].grfeopid = 2176;
	g_aeopid[261].otyp = OTYP_Bool;
	g_aeopid[261].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BUTTON, btn) + offsetof(BTN, fReapplyAseg));
	g_aeopid[261].optdat.fDef = 0;

	g_aeopid[262].grfeopid = 2176;
	g_aeopid[262].otyp = OTYP_Bool;
	g_aeopid[262].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BUTTON, btn) + offsetof(BTN, fManualReset));
	g_aeopid[262].optdat.fDef = 0;

	g_aeopid[263].grfeopid = 2176;
	g_aeopid[263].otyp = OTYP_Bool;
	g_aeopid[263].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BUTTON, btn) + offsetof(BTN, fCheckpointed));
	g_aeopid[263].optdat.fDef = 1;

	g_aeopid[264].otyp = OTYP_Void;
	g_aeopid[264].grfeopid = 1025;
	g_aeopid[264].optdat.pvThunkFnUser = RefThunkButtonSET_SM_GOAL;
	g_aeopid[264].optdat.ibSet = 4;
	g_aeopid[264].optdat.pvThunkFn = RefThunkButtonSET_SM_GOAL;

	g_aeopid[265].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BUTTON, btn) + offsetof(BTN, oidDialog));
	g_aeopid[265].otyp = OTYP_Oid;
	g_aeopid[265].grfeopid = 128;

	g_aeopid[266].otyp = OTYP_Jsglk | OTYP_Vector4;
	g_aeopid[266].grfeopid = 34;
	g_aeopid[266].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BUTTON, btn) + offsetof(BTN, buttons));
	g_aeopid[266].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetButtonButtons);

	g_aeopid[267].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddVolbtnAseg);
	g_aeopid[267].otyp = OTYP_Oid;
	g_aeopid[267].grfeopid = 256;

	g_aeopid[268].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddVolbtnOffAseg);
	g_aeopid[268].otyp = OTYP_Oid;
	g_aeopid[268].grfeopid = 256;

	g_aeopid[269].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(VOLBTN, btn) + offsetof(BTN, dtRepush));
	g_aeopid[269].otyp = OTYP_Float;
	g_aeopid[269].grfeopid = 2194;
	g_aeopid[269].optdat.fDef = 1065353216;
	g_aeopid[269].optdat.ibGet = static_cast<std::intptr_t>(offsetof(VOLBTN, btn) + offsetof(BTN, dtRepush));
	g_aeopid[269].optdat.ibSet = static_cast<std::intptr_t>(offsetof(VOLBTN, btn) + offsetof(BTN, dtRepush));

	g_aeopid[270].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(VOLBTN, btn) + offsetof(BTN, svtAnimation));
	g_aeopid[270].otyp = OTYP_Float;
	g_aeopid[270].grfeopid = 2176;
	g_aeopid[270].optdat.fDef = 1065353216;

	g_aeopid[271].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(VOLBTN, btn) + offsetof(BTN, fSilent));
	g_aeopid[271].grfeopid = 2176;
	g_aeopid[271].optdat.fDef = 1;
	g_aeopid[271].otyp = OTYP_Bool;

	g_aeopid[272].otyp = OTYP_Bool;
	g_aeopid[272].grfeopid = 2176;
	g_aeopid[272].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(VOLBTN, btn) + offsetof(BTN, fReapplyAseg));
	g_aeopid[272].optdat.fDef = 0;

	g_aeopid[273].otyp = OTYP_Bool;
	g_aeopid[273].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(VOLBTN, btn) + offsetof(BTN, fCheckpointed));
	g_aeopid[273].grfeopid = 2176;
	g_aeopid[273].optdat.fDef = 0;

	g_aeopid[274].grfeopid = 2194;
	g_aeopid[274].otyp = OTYP_Bool;
	g_aeopid[274].optdat.ibGet = static_cast<std::intptr_t>(offsetof(VOLBTN, fMustExit));
	g_aeopid[274].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(VOLBTN, fMustExit));
	g_aeopid[274].optdat.ibSet = static_cast<std::intptr_t>(offsetof(VOLBTN, fMustExit));
	g_aeopid[274].optdat.fDef = 1;

	g_aeopid[275].grfeopid = 256;
	g_aeopid[275].otyp = OTYP_Oid;
	g_aeopid[275].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddVolbtnPushObject);

	g_aeopid[276].grfeopid = 1025;
	g_aeopid[276].optdat.pvThunkFnUser = RefThunkVolbtnSET_SM_GOAL;
	g_aeopid[276].otyp = OTYP_Void;
	g_aeopid[276].optdat.ibSet = 4;
	g_aeopid[276].optdat.pvThunkFn = RefThunkVolbtnSET_SM_GOAL;

	g_aeopid[277].grfeopid = 2194;
	g_aeopid[277].optdat.ibGet = static_cast<std::intptr_t>(offsetof(VOLBTN, fSendChangeEvents));
	g_aeopid[277].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(VOLBTN, fSendChangeEvents));
	g_aeopid[277].otyp = OTYP_Bool;
	g_aeopid[277].optdat.ibSet = static_cast<std::intptr_t>(offsetof(VOLBTN, fSendChangeEvents));
	g_aeopid[277].optdat.fDef = 0;

	g_aeopid[278].grfeopid = 4;
	g_aeopid[278].otyp = OTYP_List | OTYP_Lo;
	g_aeopid[278].optdat.pvThunkFnUser = (PVTHUNKFN)FGetVolbtnPushObjectsWithinList;

	g_aeopid[279].grfeopid = 128;
	g_aeopid[279].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(VOLBTN, btn) + offsetof(BTN, oidDialog));
	g_aeopid[279].otyp = OTYP_Oid;

	g_aeopid[280].otyp = OTYP_Jsglk | OTYP_Vector4;
	g_aeopid[280].grfeopid = 34;
	g_aeopid[280].optdat.ibGet = static_cast<std::intptr_t>(offsetof(VOLBTN, btn) + offsetof(BTN, buttons));
	g_aeopid[280].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetVolbtnButtons);

	g_aeopid[281].grfeopid = 290;
	g_aeopid[281].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetPoPlayable);
	g_aeopid[281].otyp = OTYP_Bool;
	g_aeopid[281].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PO, fPlayable));
	g_aeopid[281].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetPoPlayable);

	g_aeopid[282].otyp = OTYP_Void;
	g_aeopid[282].grfeopid = 1;
	g_aeopid[282].optdat.pvThunkFn = RefThunkPoMAKE_ACTIVE;
	g_aeopid[282].optdat.ibGet = 0;
	g_aeopid[282].optdat.ibSet = 0;

	g_aeopid[283].otyp = OTYP_Bool;
	g_aeopid[283].grfeopid = 1;
	g_aeopid[283].optdat.ibSet = 1;
	g_aeopid[283].optdat.pvThunkFn = RefThunkPoINVULNERABLEQ;
	g_aeopid[283].optdat.ibGet = 0;

	g_aeopid[284].otyp = 4198;
	g_aeopid[284].grfeopid = 4;
	g_aeopid[284].optdat.pvThunkFnUser = (PVTHUNKFN)GetJthsCurrentPo;

	g_aeopid[285].optdat.pfnsetUser = (PFNRAW)AddFrzgObject;
	g_aeopid[285].otyp = OTYP_Oid;
	g_aeopid[285].grfeopid = 0x100;

	g_aeopid[286].grfeopid = 2176;
	g_aeopid[286].otyp = OTYP_Float;
	g_aeopid[286].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPRIZE, sCollect));
	g_aeopid[286].optdat.fDef = 1120403456;

	g_aeopid[287].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddSprizeAseg);
	g_aeopid[287].otyp = OTYP_Oid;
	g_aeopid[287].grfeopid = 256;

	g_aeopid[288].grfeopid = 146;
	g_aeopid[288].otyp = OTYP_Bool;
	g_aeopid[288].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SPRIZE, fNoExit));
	g_aeopid[288].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPRIZE, fNoExit));
	g_aeopid[288].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SPRIZE, fNoExit));

	g_aeopid[289].otyp = OTYP_Oid;
	g_aeopid[289].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddAlarmAlbrk);
	g_aeopid[289].grfeopid = 256;

	g_aeopid[290].otyp = OTYP_Oid;
	g_aeopid[290].grfeopid = 256;
	g_aeopid[290].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddAlarmSensor);

	g_aeopid[291].otyp = OTYP_Oid;
	g_aeopid[291].grfeopid = 256;
	g_aeopid[291].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddAlarmStepguard);

	g_aeopid[292].grfeopid = 2194;
	g_aeopid[292].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALARM, fSilent));
	g_aeopid[292].otyp = OTYP_Bool;
	g_aeopid[292].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ALARM, fSilent));
	g_aeopid[292].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ALARM, fSilent));
	g_aeopid[292].optdat.fDef = 0;

	g_aeopid[293].otyp = OTYP_Float;
	g_aeopid[293].grfeopid = 2194;
	g_aeopid[293].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALARM, dtReset));
	g_aeopid[293].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ALARM, dtReset));
	g_aeopid[293].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ALARM, dtReset));
	g_aeopid[293].optdat.fDef = 0;

	g_aeopid[294].otyp = OTYP_Sensm;
	g_aeopid[294].grfeopid = 2;
	g_aeopid[294].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALARM, alarms));

	g_aeopid[295].optdat.pvThunkFnUser = (PVTHUNKFN)FGetAlarmSensorList;
	g_aeopid[295].otyp = OTYP_List | OTYP_Sprize;
	g_aeopid[295].grfeopid = 4;

	g_aeopid[296].otyp = OTYP_Void;
	g_aeopid[296].grfeopid = 1;
	g_aeopid[296].optdat.pvThunkFn = RefThunkAlarmTRIGGER;
	g_aeopid[296].optdat.ibSet = 1;

	g_aeopid[297].otyp = OTYP_Void;
	g_aeopid[297].grfeopid = 1025;
	g_aeopid[297].optdat.pvThunkFnUser = RefThunkAlarmSET_SM_GOAL;
	g_aeopid[297].optdat.ibSet = 4;
	g_aeopid[297].optdat.pvThunkFn = RefThunkAlarmSET_SM_GOAL;

	g_aeopid[298].otyp = OTYP_Bone;
	g_aeopid[298].grfeopid = 2;
	g_aeopid[298].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ALBRK, palarm));

	g_aeopid[299].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddSensorTriggerObject);
	g_aeopid[299].otyp = OTYP_Oid;
	g_aeopid[299].grfeopid = 256;

	g_aeopid[300].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddSensorNoTriggerObject);
	g_aeopid[300].otyp = OTYP_Oid;
	g_aeopid[300].grfeopid = 256;

	g_aeopid[301].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddSensorTriggerClass);
	g_aeopid[301].otyp = OTYP_Cid;
	g_aeopid[301].grfeopid = 256;

	g_aeopid[302].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddSensorNoTriggerClass);
	g_aeopid[302].otyp = OTYP_Cid;
	g_aeopid[302].grfeopid = 256;

	g_aeopid[303].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SENSOR, fTriggerAll));
	g_aeopid[303].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SENSOR, fTriggerAll));
	g_aeopid[303].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SENSOR, fTriggerAll));
	g_aeopid[303].otyp = OTYP_Bool;
	g_aeopid[303].grfeopid = 2194;
	g_aeopid[303].optdat.fDef = 0;

	g_aeopid[304].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SENSOR, dtEnabling));
	g_aeopid[304].otyp = OTYP_Float;
	g_aeopid[304].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SENSOR, dtEnabling));
	g_aeopid[304].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SENSOR, dtEnabling));
	g_aeopid[304].grfeopid = 2194;
	g_aeopid[304].optdat.fDef = 1065353216;

	g_aeopid[305].otyp = OTYP_Float;
	g_aeopid[305].grfeopid = 2194;
	g_aeopid[305].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SENSOR, dtDisabling));
	g_aeopid[305].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SENSOR, dtDisabling));
	g_aeopid[305].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SENSOR, dtDisabling));
	g_aeopid[305].optdat.fDef = 1065353216;

	g_aeopid[306].grfeopid = 2194;
	g_aeopid[306].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SENSOR, sensorsInitial));
	g_aeopid[306].otyp = OTYP_Csdts;
	g_aeopid[306].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SENSOR, sensorsInitial));
	g_aeopid[306].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SENSOR, sensorsInitial));
	g_aeopid[306].optdat.fDef = 1;

	g_aeopid[307].grfeopid = 2;
	g_aeopid[307].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SENSOR, palarm));
	g_aeopid[307].otyp = OTYP_Bone;

	g_aeopid[308].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SENSOR, sensors));
	g_aeopid[308].grfeopid = 2;
	g_aeopid[308].otyp = OTYP_Csdts;

	g_aeopid[309].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SENSOR, sensm));
	g_aeopid[309].grfeopid = 2;
	g_aeopid[309].otyp = OTYP_Lask;

	g_aeopid[310].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SENSOR, fRemainDisabledIndefinite));
	g_aeopid[310].grfeopid = 18;
	g_aeopid[310].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SENSOR, fRemainDisabledIndefinite));
	g_aeopid[310].otyp = OTYP_Bool;

	g_aeopid[311].grfeopid = 1;
	g_aeopid[311].optdat.ibSet = 1;
	g_aeopid[311].optdat.pvThunkFn = RefThunkSensorENABLE;
	g_aeopid[311].otyp = OTYP_Void;

	g_aeopid[312].grfeopid = 1;
	g_aeopid[312].optdat.pvThunkFn = RefThunkSensorDISABLE;
	g_aeopid[312].otyp = OTYP_Void;
	g_aeopid[312].optdat.ibSet = 0;

	g_aeopid[313].otyp = OTYP_Pssat;
	g_aeopid[313].grfeopid = 2194;
	g_aeopid[313].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LASEN, lask));
	g_aeopid[313].optdat.ibSet = static_cast<std::intptr_t>(offsetof(LASEN, lask));
	g_aeopid[313].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(LASEN, lask));
	g_aeopid[313].optdat.fDef = 0;

	g_aeopid[314].grfeopid = 2194;
	g_aeopid[314].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LASEN, dtDamageDisabling));
	g_aeopid[314].otyp = OTYP_Float;
	g_aeopid[314].optdat.ibSet = static_cast<std::intptr_t>(offsetof(LASEN, dtDamageDisabling));
	g_aeopid[314].optdat.fDef = 1048576000;
	g_aeopid[314].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(LASEN, dtDamageDisabling));

	g_aeopid[315].grfeopid = 1;
	g_aeopid[315].otyp = OTYP_Void;
	g_aeopid[315].optdat.ibSet = 1;
	g_aeopid[315].optdat.pvThunkFn = RefThunkLasenEXTEND;

	g_aeopid[316].grfeopid = 1;
	g_aeopid[316].otyp = OTYP_Void;
	g_aeopid[316].optdat.ibGet = 0;
	g_aeopid[316].optdat.ibSet = 1;
	g_aeopid[316].optdat.pvThunkFn = RefThunkLasenRETRACT;

	g_aeopid[317].optdat.fDef = 1048576000;
	g_aeopid[317].grfeopid = 2194;
	g_aeopid[317].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CAMSEN, dtDamageFocus));
	g_aeopid[317].otyp = OTYP_Float;
	g_aeopid[317].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CAMSEN, dtDamageFocus));
	g_aeopid[317].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CAMSEN, dtDamageFocus));

	g_aeopid[318].optdat.fDef = 1065353216;
	g_aeopid[318].grfeopid = 2194;
	g_aeopid[318].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CAMSEN, dtDamageZap));
	g_aeopid[318].otyp = OTYP_Float;
	g_aeopid[318].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CAMSEN, dtDamageZap));
	g_aeopid[318].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CAMSEN, dtDamageZap));

	g_aeopid[319].otyp = OTYP_Float;
	g_aeopid[319].grfeopid = 2194;
	g_aeopid[319].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CAMSEN, dtDamageUnfocus));
	g_aeopid[319].optdat.fDef = 1061158912;
	g_aeopid[319].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CAMSEN, dtDamageUnfocus));
	g_aeopid[319].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CAMSEN, dtDamageUnfocus));

	g_aeopid[320].otyp = OTYP_Rchmk;
	g_aeopid[320].grfeopid = 2;
	g_aeopid[320].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CAMSEN, csdts));

	g_aeopid[321].otyp = OTYP_Int;
	g_aeopid[321].grfeopid = 146;
	g_aeopid[321].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, iframeSenseStart));
	g_aeopid[321].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, iframeSenseStart));
	g_aeopid[321].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, iframeSenseStart));

	g_aeopid[322].otyp = OTYP_Int;
	g_aeopid[322].grfeopid = 146;
	g_aeopid[322].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, iframeSenseEnd));
	g_aeopid[322].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, iframeSenseEnd));
	g_aeopid[322].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, iframeSenseEnd));

	g_aeopid[323].otyp = OTYP_Float;
	g_aeopid[323].grfeopid = 146;
	g_aeopid[323].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, dtSenseAnim));
	g_aeopid[323].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, dtSenseAnim));
	g_aeopid[323].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, dtSenseAnim));

	g_aeopid[324].otyp = OTYP_Darts;
	g_aeopid[324].grfeopid = 2194;
	g_aeopid[324].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, pssatSense));
	g_aeopid[324].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, pssatSense));
	g_aeopid[324].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, pssatSense));
	g_aeopid[324].optdat.fDef = 0;

	g_aeopid[325].otyp = OTYP_Int;
	g_aeopid[325].grfeopid = 146;
	g_aeopid[325].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDamageStart));
	g_aeopid[325].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDamageStart));
	g_aeopid[325].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDamageStart));

	g_aeopid[326].otyp = OTYP_Int;
	g_aeopid[326].grfeopid = 146;
	g_aeopid[326].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDamageEnd));
	g_aeopid[326].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDamageEnd));
	g_aeopid[326].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDamageEnd));

	g_aeopid[327].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, dtDamageAnim));
	g_aeopid[327].otyp = OTYP_Float;
	g_aeopid[327].grfeopid = 146;
	g_aeopid[327].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, dtDamageAnim));
	g_aeopid[327].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, dtDamageAnim));

	g_aeopid[328].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, pssatDamage));
	g_aeopid[328].otyp = OTYP_Darts;
	g_aeopid[328].grfeopid = 2194;
	g_aeopid[328].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, pssatDamage));
	g_aeopid[328].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, pssatDamage));
	g_aeopid[328].optdat.fDef = 0;

	g_aeopid[329].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDisabledStart));
	g_aeopid[329].otyp = OTYP_Int;
	g_aeopid[329].grfeopid = 146;
	g_aeopid[329].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDisabledStart));
	g_aeopid[329].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDisabledStart));

	g_aeopid[330].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDisabledEnd));
	g_aeopid[330].otyp = OTYP_Int;
	g_aeopid[330].grfeopid = 146;
	g_aeopid[330].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDisabledEnd));
	g_aeopid[330].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDisabledEnd));

	g_aeopid[331].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, dtDisabledAnim));
	g_aeopid[331].otyp = OTYP_Float;
	g_aeopid[331].grfeopid = 146;
	g_aeopid[331].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, dtDisabledAnim));
	g_aeopid[331].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, dtDisabledAnim));

	g_aeopid[332].otyp = OTYP_Darts;
	g_aeopid[332].grfeopid = 2194;
	g_aeopid[332].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, pssatDisabled));
	g_aeopid[332].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, pssatDisabled));
	g_aeopid[332].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, pssatDisabled));
	g_aeopid[332].optdat.fDef = 0;

	g_aeopid[333].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDisablingFlash));
	g_aeopid[333].otyp = OTYP_Int;
	g_aeopid[333].grfeopid = 146;
	g_aeopid[333].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDisablingFlash));
	g_aeopid[333].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, iframeDisablingFlash));

	g_aeopid[334].otyp = OTYP_Float;
	g_aeopid[334].grfeopid = 2194;
	g_aeopid[334].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, svtDisablingFlash));
	g_aeopid[334].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, svtDisablingFlash));
	g_aeopid[334].optdat.fDef = 1090519040;
	g_aeopid[334].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, svtDisablingFlash));

	g_aeopid[335].otyp = OTYP_Float;
	g_aeopid[335].grfeopid = 2194;
	g_aeopid[335].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, dtRemainEnabled));
	g_aeopid[335].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, dtRemainEnabled));
	g_aeopid[335].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, dtRemainEnabled));
	g_aeopid[335].optdat.fDef = 1073741824;

	g_aeopid[336].grfeopid = 2194;
	g_aeopid[336].otyp = OTYP_Float;
	g_aeopid[336].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PRSEN, dtRemainDisabled));
	g_aeopid[336].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PRSEN, dtRemainDisabled));
	g_aeopid[336].optdat.fDef = 1073741824;
	g_aeopid[336].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PRSEN, dtRemainDisabled));

	g_aeopid[337].otyp = OTYP_Axis;
	g_aeopid[337].grfeopid = 130;
	g_aeopid[337].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HND, hndk));
	g_aeopid[337].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HND, hndk));

	g_aeopid[338].otyp = OTYP_Float;
	g_aeopid[338].grfeopid = 146;
	g_aeopid[338].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HND, svzJumpBoost));
	g_aeopid[338].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HND, svzJumpBoost));
	g_aeopid[338].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HND, svzJumpBoost));

	g_aeopid[339].otyp = OTYP_Sfxid;
	g_aeopid[339].grfeopid = 146;
	g_aeopid[339].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HND, sfxidGrab));
	g_aeopid[339].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HND, sfxidGrab));
	g_aeopid[339].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HND, sfxidGrab));

	g_aeopid[340].otyp = OTYP_Sfxid;
	g_aeopid[340].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HND, sfxidRelease));
	g_aeopid[340].grfeopid = 146;
	g_aeopid[340].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HND, sfxidRelease));
	g_aeopid[340].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HND, sfxidRelease));

	g_aeopid[341].otyp = OTYP_Float;
	g_aeopid[341].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HND, sRadiusHook));
	g_aeopid[341].grfeopid = 146;
	g_aeopid[341].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HND, sRadiusHook));
	g_aeopid[341].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HND, sRadiusHook));

	g_aeopid[342].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HND, unkFloat1));
	g_aeopid[342].otyp = OTYP_Bool;
	g_aeopid[342].grfeopid = 146;
	g_aeopid[342].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HND, unkFloat1));
	g_aeopid[342].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HND, unkFloat1));

	g_aeopid[343].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HND, dzJumpTargetMax));
	g_aeopid[343].otyp = OTYP_Float;
	g_aeopid[343].grfeopid = 146;
	g_aeopid[343].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HND, dzJumpTargetMax));
	g_aeopid[343].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HND, dzJumpTargetMax));

	g_aeopid[344].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddBarrierObject);
	g_aeopid[344].otyp = OTYP_Oid;
	g_aeopid[344].grfeopid = 256;

	g_aeopid[345].otyp = OTYP_Cid;
	g_aeopid[345].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddBarrierClass);
	g_aeopid[345].grfeopid = 256;

	g_aeopid[346].otyp = OTYP_Bool;
	g_aeopid[346].grfeopid = 128;
	g_aeopid[346].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BARRIER, fAllow));

	g_aeopid[347].otyp = OTYP_Oid;
	g_aeopid[347].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BARRIER, barwarp) + offsetof(BARWARP, oidWarp));
	g_aeopid[347].grfeopid = 128;

	g_aeopid[348].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetEmitterParticleCount);
	g_aeopid[348].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetEmitterParticleCount);
	g_aeopid[348].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITTER, cParticle));
	g_aeopid[348].otyp = OTYP_Int;
	g_aeopid[348].grfeopid = 290;

	g_aeopid[349].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetEmitterAutoPause);
	g_aeopid[349].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetEmitterAutoPause);
	g_aeopid[349].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITTER, fAutoPause));
	g_aeopid[349].grfeopid = 290;
	g_aeopid[349].otyp = OTYP_Bool;

	g_aeopid[350].otyp = OTYP_Emitrk;
	g_aeopid[350].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITTER, emitrk));
	g_aeopid[350].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITTER, emitrk));
	g_aeopid[350].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITTER, emitrk));
	g_aeopid[350].grfeopid = 4242;
	g_aeopid[350].optdat.pfnensure = (PFNENSURE)PemitterEnsureEmitter;

	g_aeopid[351].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITTER, lmSvcParticle));
	g_aeopid[351].otyp = OTYP_Lm;
	g_aeopid[351].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITTER, lmSvcParticle));
	g_aeopid[351].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITTER, lmSvcParticle));
	g_aeopid[351].grfeopid = 4242;
	g_aeopid[351].optdat.pfnensure = (PFNENSURE)PemitterEnsureEmitter;

	g_aeopid[352].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITTER, cParticleConstant));
	g_aeopid[352].otyp = OTYP_Float;
	g_aeopid[352].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITTER, cParticleConstant));
	g_aeopid[352].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITTER, cParticleConstant));
	g_aeopid[352].grfeopid = 4242;
	g_aeopid[352].optdat.pfnensure = (PFNENSURE)PemitterEnsureEmitter;

	g_aeopid[353].otyp = OTYP_Float;
	g_aeopid[353].grfeopid = 4242;
	g_aeopid[353].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITTER, uPauseProb));
	g_aeopid[353].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITTER, uPauseProb));
	g_aeopid[353].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITTER, uPauseProb));
	g_aeopid[353].optdat.pfnensure = (PFNENSURE)PemitterEnsureEmitter;

	g_aeopid[354].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITTER, lmDtPause));
	g_aeopid[354].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITTER, lmDtPause));
	g_aeopid[354].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITTER, lmDtPause));
	g_aeopid[354].optdat.pfnensure = (PFNENSURE)PemitterEnsureEmitter;
	g_aeopid[354].otyp = OTYP_Lm;
	g_aeopid[354].grfeopid = 4242;

	g_aeopid[355].grfeopid = 36;
	g_aeopid[355].optdat.pvThunkFnUser = (PVTHUNKFN)GetEmitterEnabled;
	g_aeopid[355].otyp = OTYP_Bool;
	g_aeopid[355].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetEmitterEnabled);

	g_aeopid[356].grfeopid = 130;
	g_aeopid[356].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITTER, fCountIsDensity));
	g_aeopid[356].otyp = OTYP_Bool;
	g_aeopid[356].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITTER, fCountIsDensity));

	g_aeopid[357].otyp = OTYP_Oid;
	g_aeopid[357].grfeopid = 130;
	g_aeopid[357].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITTER, oidReference));
	g_aeopid[357].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITTER, oidReference));

	g_aeopid[358].otyp = OTYP_Oid;
	g_aeopid[358].grfeopid = 130;
	g_aeopid[358].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITTER, oidRender));
	g_aeopid[358].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITTER, oidRender));

	g_aeopid[359].otyp = OTYP_Oid;
	g_aeopid[359].grfeopid = 130;
	g_aeopid[359].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITTER, oidTouch));
	g_aeopid[359].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITTER, oidTouch));

	g_aeopid[360].grfeopid = 130;
	g_aeopid[360].otyp = OTYP_Oid;
	g_aeopid[360].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITTER, oidNextRender));
	g_aeopid[360].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITTER, oidNextRender));

	g_aeopid[361].otyp = OTYP_Oid;
	g_aeopid[361].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITTER, oidGroup));
	g_aeopid[361].grfeopid = 130;
	g_aeopid[361].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITTER, oidGroup));

	g_aeopid[362].otyp = OTYP_Void;
	g_aeopid[362].grfeopid = 1;
	g_aeopid[362].optdat.ibGet = 0;
	g_aeopid[362].optdat.pvThunkFn = RefThunkEmitterPAUSE;
	g_aeopid[362].optdat.ibSet = 1;

	g_aeopid[363].otyp = OTYP_Void;
	g_aeopid[363].grfeopid = 1;
	g_aeopid[363].optdat.ibGet = 0;
	g_aeopid[363].optdat.pvThunkFn = RefThunkEmitterPAUSE_FOREVER;
	g_aeopid[363].optdat.ibSet = 0;

	g_aeopid[364].optdat.pvThunkFn = RefThunkEmitterUNPAUSE;
	g_aeopid[364].otyp = OTYP_Void;
	g_aeopid[364].grfeopid = 1;
	g_aeopid[364].optdat.ibGet = 0;
	g_aeopid[364].optdat.ibSet = 0;

	g_aeopid[365].optdat.pvThunkFnUser = (PVTHUNKFN)GetEmitterPaused;
	g_aeopid[365].grfeopid = 4;
	g_aeopid[365].otyp = OTYP_Bool;

	g_aeopid[366].otyp = OTYP_Emitok;
	g_aeopid[366].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitok));
	g_aeopid[366].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitok));
	g_aeopid[366].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;
	g_aeopid[366].grfeopid = 4226;

	g_aeopid[367].otyp = OTYP_Vector;
	g_aeopid[367].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, posOrigin));
	g_aeopid[367].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, posOrigin));
	g_aeopid[367].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;
	g_aeopid[367].grfeopid = 4226;

	g_aeopid[368].otyp = OTYP_Vector;
	g_aeopid[368].grfeopid = 4226;
	g_aeopid[368].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, posOrigin));
	g_aeopid[368].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, posOrigin));
	g_aeopid[368].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[369].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, boxOrigin) + offsetof(BOX, posMax));
	g_aeopid[369].otyp = OTYP_Vector;
	g_aeopid[369].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, boxOrigin) + offsetof(BOX, posMax));
	g_aeopid[369].grfeopid = 4226;
	g_aeopid[369].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[370].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITTER, oidShape));
	g_aeopid[370].otyp = OTYP_Oid;
	g_aeopid[370].grfeopid = 130;
	g_aeopid[370].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITTER, oidShape));

	g_aeopid[371].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitnk));
	g_aeopid[371].otyp = OTYP_Emitnk;
	g_aeopid[371].grfeopid = 4242;
	g_aeopid[371].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitnk));
	g_aeopid[371].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;
	g_aeopid[371].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitnk));

	g_aeopid[372].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[372].otyp = OTYP_Vector;
	g_aeopid[372].grfeopid = 4242;
	g_aeopid[372].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[372].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;
	g_aeopid[372].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));

	g_aeopid[373].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[373].otyp = OTYP_Vector;
	g_aeopid[373].grfeopid = 4242;
	g_aeopid[373].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[373].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[373].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[374].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[374].otyp = OTYP_Vector;
	g_aeopid[374].grfeopid = 4242;
	g_aeopid[374].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[374].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[374].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[375].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, lmSOffset));
	g_aeopid[375].otyp = OTYP_Lm;
	g_aeopid[375].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, lmSOffset));
	g_aeopid[375].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, lmSOffset));
	g_aeopid[375].grfeopid = 4242;
	g_aeopid[375].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[376].otyp = OTYP_Emitvk;
	g_aeopid[376].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitvk));
	g_aeopid[376].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitvk));
	g_aeopid[376].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitvk));
	g_aeopid[376].grfeopid = 4242;
	g_aeopid[376].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[377].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmSv));
	g_aeopid[377].otyp = OTYP_Lm;
	g_aeopid[377].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmSv));
	g_aeopid[377].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmSv));
	g_aeopid[377].grfeopid = 4242;
	g_aeopid[377].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[378].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, rSvz));
	g_aeopid[378].otyp = OTYP_Float;
	g_aeopid[378].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, rSvz));
	g_aeopid[378].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, rSvz));
	g_aeopid[378].grfeopid = 4242;
	g_aeopid[378].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[379].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, dv));
	g_aeopid[379].otyp = OTYP_Vector;
	g_aeopid[379].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, dv));
	g_aeopid[379].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, dv));
	g_aeopid[379].grfeopid = 4242;
	g_aeopid[379].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[380].otyp = OTYP_Float;
	g_aeopid[380].grfeopid = 4242;
	g_aeopid[380].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, rvDamping));
	g_aeopid[380].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, rvDamping));
	g_aeopid[380].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, rvDamping));
	g_aeopid[380].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[381].otyp = OTYP_Float;
	g_aeopid[381].grfeopid = 4242;
	g_aeopid[381].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, swCurl));
	g_aeopid[381].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, swCurl));
	g_aeopid[381].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, swCurl));
	g_aeopid[381].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[382].otyp = OTYP_Vector;
	g_aeopid[382].grfeopid = 4242;
	g_aeopid[382].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, normalCurl));
	g_aeopid[382].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, normalCurl));
	g_aeopid[382].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, normalCurl));
	g_aeopid[382].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[383].otyp = OTYP_Emitcnk;
	g_aeopid[383].grfeopid = 4242;
	g_aeopid[383].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, emitcnk));
	g_aeopid[383].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, emitcnk));
	g_aeopid[383].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, emitcnk));
	g_aeopid[383].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[384].otyp = OTYP_Lm;
	g_aeopid[384].grfeopid = 4242;
	g_aeopid[384].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmTilt));
	g_aeopid[384].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmTilt));
	g_aeopid[384].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmTilt));
	g_aeopid[384].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[385].otyp = OTYP_Int;
	g_aeopid[385].grfeopid = 4242;
	g_aeopid[385].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, cParticlePerRing));
	g_aeopid[385].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, cParticlePerRing));
	g_aeopid[385].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, cParticlePerRing));
	g_aeopid[385].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[386].otyp = OTYP_Float;
	g_aeopid[386].grfeopid = 4242;
	g_aeopid[386].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, uRandomRad));
	g_aeopid[386].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, uRandomRad));
	g_aeopid[386].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, uRandomRad));
	g_aeopid[386].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[387].grfeopid = 4242;
	g_aeopid[387].otyp = OTYP_Float;
	g_aeopid[387].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, dtLifetime));
	g_aeopid[387].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, dtLifetime));
	g_aeopid[387].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, dtLifetime));
	g_aeopid[387].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[388].grfeopid = 4242;
	g_aeopid[388].otyp = OTYP_Lm;
	g_aeopid[388].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmDtSkip));
	g_aeopid[388].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmDtSkip));
	g_aeopid[388].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmDtSkip));
	g_aeopid[388].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[389].grfeopid = 4242;
	g_aeopid[389].otyp = OTYP_Lm;
	g_aeopid[389].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmDtBirth));
	g_aeopid[389].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmDtBirth));
	g_aeopid[389].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmDtBirth));
	g_aeopid[389].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[390].grfeopid = 4242;
	g_aeopid[390].otyp = OTYP_Clq;
	g_aeopid[390].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, clqAlpha));
	g_aeopid[390].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, clqAlpha));
	g_aeopid[390].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, clqAlpha));
	g_aeopid[390].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[391].grfeopid = 4226;
	g_aeopid[391].otyp = OTYP_Emitpk;
	g_aeopid[391].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitpk));
	g_aeopid[391].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitpk));
	g_aeopid[391].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[392].grfeopid = 256;
	g_aeopid[392].otyp = OTYP_Ript;
	g_aeopid[392].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetEmitterRipt);

	g_aeopid[393].otyp = OTYP_Ript;
	g_aeopid[393].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, riptTrail));
	g_aeopid[393].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, riptTrail));
	g_aeopid[393].grfeopid = 4226;
	g_aeopid[393].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[394].otyp = OTYP_Lm;
	g_aeopid[394].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, lmGScale));
	g_aeopid[394].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, lmGScale));
	g_aeopid[394].grfeopid = 4242;
	g_aeopid[394].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, lmGScale));
	g_aeopid[394].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[395].otyp = OTYP_Clq;
	g_aeopid[395].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, clqScale));
	g_aeopid[395].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, clqScale));
	g_aeopid[395].grfeopid = 4242;
	g_aeopid[395].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, clqScale));
	g_aeopid[395].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[396].otyp = OTYP_Ripgt;
	g_aeopid[396].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, ripgt));
	g_aeopid[396].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, ripgt));
	g_aeopid[396].grfeopid = 4226;
	g_aeopid[396].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[397].otyp = OTYP_Float;
	g_aeopid[397].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, sExpand));
	g_aeopid[397].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, sExpand));
	g_aeopid[397].grfeopid = 4242;
	g_aeopid[397].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, sExpand));
	g_aeopid[397].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[398].otyp = OTYP_Oid;
	g_aeopid[398].grfeopid = 4226;
	g_aeopid[398].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, oidShader));
	g_aeopid[398].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, oidShader));
	g_aeopid[398].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[399].otyp = OTYP_Clq;
	g_aeopid[399].grfeopid = 4242;
	g_aeopid[399].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqScale));
	g_aeopid[399].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqScale));
	g_aeopid[399].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqScale));
	g_aeopid[399].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[400].grfeopid = 4242;
	g_aeopid[400].otyp = OTYP_Clq;
	g_aeopid[400].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqTexture));
	g_aeopid[400].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqTexture));
	g_aeopid[400].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqTexture));
	g_aeopid[400].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[401].grfeopid = 4242;
	g_aeopid[401].otyp = OTYP_Clq;
	g_aeopid[401].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqColor));
	g_aeopid[401].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqColor));
	g_aeopid[401].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqColor));
	g_aeopid[401].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[402].grfeopid = 4242;
	g_aeopid[402].otyp = OTYP_Bool;
	g_aeopid[402].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fShaderSpan));
	g_aeopid[402].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fShaderSpan));
	g_aeopid[402].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fShaderSpan));
	g_aeopid[402].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[403].grfeopid = 4242;
	g_aeopid[403].otyp = OTYP_Float;
	g_aeopid[403].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, dtShaderLoop));
	g_aeopid[403].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, dtShaderLoop));
	g_aeopid[403].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, dtShaderLoop));
	g_aeopid[403].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[404].otyp = OTYP_Bool;
	g_aeopid[404].grfeopid = 4242;
	g_aeopid[404].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fRandomFrame));
	g_aeopid[404].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fRandomFrame));
	g_aeopid[404].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fRandomFrame));
	g_aeopid[404].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[405].otyp = OTYP_Blipmk;
	g_aeopid[405].grfeopid = 4242;
	g_aeopid[405].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, blipmk));
	g_aeopid[405].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, blipmk));
	g_aeopid[405].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, blipmk));
	g_aeopid[405].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[406].otyp = OTYP_Oid;
	g_aeopid[406].grfeopid = 4226;
	g_aeopid[406].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, oidSplineTarget));
	g_aeopid[406].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, oidSplineTarget));
	g_aeopid[406].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[407].otyp = OTYP_Blipok;
	g_aeopid[407].grfeopid = 4242;
	g_aeopid[407].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, blipok));
	g_aeopid[407].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, blipok));
	g_aeopid[407].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, blipok));
	g_aeopid[407].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[408].otyp = OTYP_Float;
	g_aeopid[408].grfeopid = 4242;
	g_aeopid[408].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, rSFlying));
	g_aeopid[408].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, rSFlying));
	g_aeopid[408].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, rSFlying));
	g_aeopid[408].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[409].otyp = OTYP_Bliprk;
	g_aeopid[409].grfeopid = 4242;
	g_aeopid[409].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, bliprk));
	g_aeopid[409].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, bliprk));
	g_aeopid[409].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, bliprk));
	g_aeopid[409].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[410].otyp = OTYP_Lm;
	g_aeopid[410].grfeopid = 4242;
	g_aeopid[410].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, lmSw));
	g_aeopid[410].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, lmSw));
	g_aeopid[410].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, lmSw));
	g_aeopid[410].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[411].otyp = OTYP_Bool;
	g_aeopid[411].grfeopid = 4242;
	g_aeopid[411].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fRandomRoll));
	g_aeopid[411].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fRandomRoll));
	g_aeopid[411].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fRandomRoll));
	g_aeopid[411].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[412].optdat.pvThunkFnUser = RefThunkEmitterADD_SKELETON;
	g_aeopid[412].otyp = OTYP_Void;
	g_aeopid[412].grfeopid = 1024;
	g_aeopid[412].optdat.ibSet = 6;
	g_aeopid[412].optdat.pvThunkFn = nullptr;

	g_aeopid[413].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, bullet) + offsetof(EMITX__bullet, fDamage));
	g_aeopid[413].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, bullet) + offsetof(EMITX__bullet, fDamage));
	g_aeopid[413].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;
	g_aeopid[413].otyp = OTYP_Bool;
	g_aeopid[413].grfeopid = 4226;

	g_aeopid[414].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, bullet) + offsetof(EMITX__bullet, fRichochet));
	g_aeopid[414].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, bullet) + offsetof(EMITX__bullet, fRichochet));
	g_aeopid[414].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;
	g_aeopid[414].otyp = OTYP_Bool;
	g_aeopid[414].grfeopid = 4226;

	g_aeopid[415].otyp = OTYP_Oid;
	g_aeopid[415].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, remit) + offsetof(EMITX__remit, oidExpls));
	g_aeopid[415].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, remit) + offsetof(EMITX__remit, oidExpls));
	g_aeopid[415].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;
	g_aeopid[415].grfeopid = 4226;

	g_aeopid[416].otyp = OTYP_Float;
	g_aeopid[416].grfeopid = 4226;
	g_aeopid[416].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, remit) + offsetof(EMITX__remit, svcParticle));
	g_aeopid[416].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, remit) + offsetof(EMITX__remit, svcParticle));
	g_aeopid[416].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;

	g_aeopid[417].otyp = OTYP_Float;
	g_aeopid[417].grfeopid = 4226;
	g_aeopid[417].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, shrapnel) + offsetof(EMITX__shrapnel, sRadius));
	g_aeopid[417].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;
	g_aeopid[417].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, shrapnel) + offsetof(EMITX__shrapnel, sRadius));

	g_aeopid[418].otyp = OTYP_Float;
	g_aeopid[418].grfeopid = 4226;
	g_aeopid[418].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, shrapnel) + offsetof(EMITX__shrapnel, elas));
	g_aeopid[418].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;
	g_aeopid[418].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, shrapnel) + offsetof(EMITX__shrapnel, elas));

	g_aeopid[419].otyp = OTYP_Float;
	g_aeopid[419].grfeopid = 4226;
	g_aeopid[419].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, shrapnel) + offsetof(EMITX__shrapnel, mu));
	g_aeopid[419].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;
	g_aeopid[419].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, shrapnel) + offsetof(EMITX__shrapnel, mu));

	g_aeopid[420].grfeopid = 4226;
	g_aeopid[420].optdat.pfnensure = (PFNENSURE)PemitbEnsureEmitter;
	g_aeopid[420].otyp = OTYP_Bool;
	g_aeopid[420].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, bullet) + offsetof(EMITX__bullet, fDamage));
	g_aeopid[420].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, bullet) + offsetof(EMITX__bullet, fDamage));


	g_aeopid[421].otyp = OTYP_Emitnk;
	g_aeopid[421].grfeopid = 4242;
	g_aeopid[421].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitok));
	g_aeopid[421].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitok));
	g_aeopid[421].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitok));
	g_aeopid[421].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[422].otyp = OTYP_Vector;
	g_aeopid[422].grfeopid = 4242;
	g_aeopid[422].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, posOrigin));
	g_aeopid[422].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, posOrigin));
	g_aeopid[422].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, posOrigin));
	g_aeopid[422].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[423].otyp = OTYP_Vector;
	g_aeopid[423].grfeopid = 4242;
	g_aeopid[423].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, posOrigin));
	g_aeopid[423].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, posOrigin));
	g_aeopid[423].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, posOrigin));
	g_aeopid[423].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[424].otyp = OTYP_Vector;
	g_aeopid[424].grfeopid = 4242;
	g_aeopid[424].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, boxOrigin) + offsetof(BOX, posMax));
	g_aeopid[424].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, boxOrigin) + offsetof(BOX, posMax));
	g_aeopid[424].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, boxOrigin) + offsetof(BOX, posMax));
	g_aeopid[424].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[425].grfeopid = 130;
	g_aeopid[425].otyp = OTYP_Oid;
	g_aeopid[425].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXPLO, oidReference));
	g_aeopid[425].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXPLO, oidReference));

	g_aeopid[426].grfeopid = 130;
	g_aeopid[426].otyp = OTYP_Oid;
	g_aeopid[426].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXPLO, oidShape));
	g_aeopid[426].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXPLO, oidShape));

	g_aeopid[427].otyp = OTYP_Emitcnk;
	g_aeopid[427].grfeopid = 4242;
	g_aeopid[427].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitnk));
	g_aeopid[427].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitnk));
	g_aeopid[427].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitnk));
	g_aeopid[427].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[428].otyp = OTYP_Vector;
	g_aeopid[428].grfeopid = 4242;
	g_aeopid[428].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[428].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[428].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[428].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[429].otyp = OTYP_Vector;
	g_aeopid[429].grfeopid = 4242;
	g_aeopid[429].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[429].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[429].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[429].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[430].otyp = OTYP_Vector;
	g_aeopid[430].grfeopid = 4242;
	g_aeopid[430].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[430].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[430].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, vec));
	g_aeopid[430].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[431].otyp = OTYP_Lm;
	g_aeopid[431].grfeopid = 4242;
	g_aeopid[431].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, lmSOffset));
	g_aeopid[431].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, lmSOffset));
	g_aeopid[431].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, lmSOffset));
	g_aeopid[431].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[432].otyp = OTYP_Lightk;
	g_aeopid[432].grfeopid = 4242;
	g_aeopid[432].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitvk));
	g_aeopid[432].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitvk));
	g_aeopid[432].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emito) + offsetof(EMITO, emitvk));
	g_aeopid[432].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[433].otyp = OTYP_Lm;
	g_aeopid[433].grfeopid = 4242;
	g_aeopid[433].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmSv));
	g_aeopid[433].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmSv));
	g_aeopid[433].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmSv));
	g_aeopid[433].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[434].otyp = OTYP_Float;
	g_aeopid[434].grfeopid = 4242;
	g_aeopid[434].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, rSvz));
	g_aeopid[434].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, rSvz));
	g_aeopid[434].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, rSvz));
	g_aeopid[434].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[435].otyp = OTYP_Lm;
	g_aeopid[435].grfeopid = 4242;
	g_aeopid[435].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmTilt));
	g_aeopid[435].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmTilt));
	g_aeopid[435].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmTilt));
	g_aeopid[435].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[436].otyp = OTYP_Int;
	g_aeopid[436].grfeopid = 4242;
	g_aeopid[436].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, cParticlePerRing));
	g_aeopid[436].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, cParticlePerRing));
	g_aeopid[436].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, cParticlePerRing));
	g_aeopid[436].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[437].otyp = OTYP_Float;
	g_aeopid[437].grfeopid = 4242;
	g_aeopid[437].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, uRandomRad));
	g_aeopid[437].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, uRandomRad));
	g_aeopid[437].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, uRandomRad));
	g_aeopid[437].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[438].otyp = OTYP_Lm;
	g_aeopid[438].grfeopid = 4242;
	g_aeopid[438].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmDtSkip));
	g_aeopid[438].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmDtSkip));
	g_aeopid[438].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmDtSkip));
	g_aeopid[438].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[439].otyp = OTYP_Void;
	g_aeopid[439].grfeopid = 1024;
	g_aeopid[439].optdat.pvThunkFnUser = RefThunkExploADD_SKELETON;
	g_aeopid[439].optdat.ibSet = 6;
	g_aeopid[439].optdat.pvThunkFn = nullptr;

	g_aeopid[440].otyp = OTYP_Lm;
	g_aeopid[440].grfeopid = 146;
	g_aeopid[440].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXPLS, lmcParticle));
	g_aeopid[440].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EXPLS, lmcParticle));
	g_aeopid[440].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXPLS, lmcParticle));

	g_aeopid[441].otyp = OTYP_Oid;
	g_aeopid[441].grfeopid = 130;
	g_aeopid[441].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXPLS, oidRender));
	g_aeopid[441].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXPLS, oidRender));

	g_aeopid[442].otyp = OTYP_Oid;
	g_aeopid[442].grfeopid = 130;
	g_aeopid[442].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXPLS, oidTouch));
	g_aeopid[442].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXPLS, oidTouch));

	g_aeopid[443].otyp = OTYP_Oid;
	g_aeopid[443].grfeopid = 130;
	g_aeopid[443].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXPLS, oidNextRender));
	g_aeopid[443].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXPLS, oidNextRender));

	g_aeopid[444].otyp = OTYP_Float;
	g_aeopid[444].grfeopid = 146;
	g_aeopid[444].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXPLS, dtDelay));
	g_aeopid[444].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EXPLS, dtDelay));
	g_aeopid[444].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXPLS, dtDelay));

	g_aeopid[445].otyp = OTYP_Bool;
	g_aeopid[445].grfeopid = 146;
	g_aeopid[445].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EXPLS, fGrouped));
	g_aeopid[445].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EXPLS, fGrouped));
	g_aeopid[445].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EXPLS, fGrouped));

	g_aeopid[446].otyp = OTYP_Vector;
	g_aeopid[446].grfeopid = 4242;
	g_aeopid[446].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, dv));
	g_aeopid[446].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, dv));
	g_aeopid[446].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, dv));
	g_aeopid[446].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[447].otyp = OTYP_Float;
	g_aeopid[447].grfeopid = 4242;
	g_aeopid[447].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, rvDamping));
	g_aeopid[447].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, rvDamping));
	g_aeopid[447].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, rvDamping));
	g_aeopid[447].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[448].otyp = OTYP_Float;
	g_aeopid[448].grfeopid = 4242;
	g_aeopid[448].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, swCurl));
	g_aeopid[448].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, swCurl));
	g_aeopid[448].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, swCurl));
	g_aeopid[448].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[449].otyp = OTYP_Vector;
	g_aeopid[449].grfeopid = 4242;
	g_aeopid[449].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, normalCurl));
	g_aeopid[449].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, normalCurl));
	g_aeopid[449].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, normalCurl));
	g_aeopid[449].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[450].otyp = OTYP_Emitvk;
	g_aeopid[450].grfeopid = 4242;
	g_aeopid[450].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, emitcnk));
	g_aeopid[450].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, emitcnk));
	g_aeopid[450].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, emitcnk));
	g_aeopid[450].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[451].otyp = OTYP_Float;
	g_aeopid[451].grfeopid = 4242;
	g_aeopid[451].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, dtLifetime));
	g_aeopid[451].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, dtLifetime));
	g_aeopid[451].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, dtLifetime));
	g_aeopid[451].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[452].otyp = OTYP_Lm;
	g_aeopid[452].grfeopid = 4242;
	g_aeopid[452].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmDtBirth));
	g_aeopid[452].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmDtBirth));
	g_aeopid[452].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, lmDtBirth));
	g_aeopid[452].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[453].otyp = OTYP_Clq;
	g_aeopid[453].grfeopid = 4242;
	g_aeopid[453].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, clqAlpha));
	g_aeopid[453].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, clqAlpha));
	g_aeopid[453].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitv) + offsetof(EMITV, clqAlpha));
	g_aeopid[453].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[454].otyp = OTYP_Emitpk;
	g_aeopid[454].grfeopid = 4226;
	g_aeopid[454].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitpk));
	g_aeopid[454].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitpk));
	g_aeopid[454].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[455].grfeopid = 256;
	g_aeopid[455].otyp = OTYP_Ript;
	g_aeopid[455].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetExploRipt);

	g_aeopid[456].otyp = OTYP_Ript;
	g_aeopid[456].grfeopid = 4226;
	g_aeopid[456].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, riptTrail));
	g_aeopid[456].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, riptTrail));
	g_aeopid[456].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[457].otyp = OTYP_Lm;
	g_aeopid[457].grfeopid = 4242;
	g_aeopid[457].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, lmGScale));
	g_aeopid[457].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, lmGScale));
	g_aeopid[457].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, lmGScale));
	g_aeopid[457].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[458].otyp = OTYP_Clq;
	g_aeopid[458].grfeopid = 4242;
	g_aeopid[458].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, clqScale));
	g_aeopid[458].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, clqScale));
	g_aeopid[458].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, clqScale));
	g_aeopid[458].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[459].otyp = OTYP_Ripgt;
	g_aeopid[459].grfeopid = 4226;
	g_aeopid[459].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, ripgt));
	g_aeopid[459].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, ripgt));
	g_aeopid[459].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[460].otyp = OTYP_Float;
	g_aeopid[460].grfeopid = 4242;
	g_aeopid[460].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, sExpand));
	g_aeopid[460].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, sExpand));
	g_aeopid[460].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitrip) + offsetof(EMITRIP, sExpand));
	g_aeopid[460].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[461].otyp = OTYP_Oid;
	g_aeopid[461].grfeopid = 4226;
	g_aeopid[461].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, oidShader));
	g_aeopid[461].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, oidShader));
	g_aeopid[461].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[462].otyp = OTYP_Clq;
	g_aeopid[462].grfeopid = 4242;
	g_aeopid[462].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqScale));
	g_aeopid[462].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqScale));
	g_aeopid[462].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqScale));
	g_aeopid[462].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[463].otyp = OTYP_Clq;
	g_aeopid[463].grfeopid = 4242;
	g_aeopid[463].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqTexture));
	g_aeopid[463].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqTexture));
	g_aeopid[463].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqTexture));
	g_aeopid[463].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[464].otyp = OTYP_Clq;
	g_aeopid[464].grfeopid = 4242;
	g_aeopid[464].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqColor));
	g_aeopid[464].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqColor));
	g_aeopid[464].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, clqColor));
	g_aeopid[464].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[465].otyp = OTYP_Bool;
	g_aeopid[465].grfeopid = 4242;
	g_aeopid[465].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fShaderSpan));
	g_aeopid[465].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fShaderSpan));
	g_aeopid[465].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fShaderSpan));
	g_aeopid[465].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[466].otyp = OTYP_Float;
	g_aeopid[466].grfeopid = 4242;
	g_aeopid[466].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, dtShaderLoop));
	g_aeopid[466].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, dtShaderLoop));
	g_aeopid[466].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, dtShaderLoop));
	g_aeopid[466].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[467].otyp = OTYP_Bool;
	g_aeopid[467].grfeopid = 4242;
	g_aeopid[467].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fRandomFrame));
	g_aeopid[467].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fRandomFrame));
	g_aeopid[467].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fRandomFrame));
	g_aeopid[467].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[468].otyp = OTYP_Bliprk;
	g_aeopid[468].grfeopid = 4242;
	g_aeopid[468].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, blipmk));
	g_aeopid[468].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, blipmk));
	g_aeopid[468].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, blipmk));
	g_aeopid[468].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[469].otyp = OTYP_Oid;
	g_aeopid[469].grfeopid = 4226;
	g_aeopid[469].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, oidSplineTarget));
	g_aeopid[469].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, oidSplineTarget));
	g_aeopid[469].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[470].otyp = OTYP_Cans;
	g_aeopid[470].grfeopid = 4242;
	g_aeopid[470].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, blipok));
	g_aeopid[470].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, blipok));
	g_aeopid[470].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, blipok));
	g_aeopid[470].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[471].otyp = OTYP_Float;
	g_aeopid[471].grfeopid = 4242;
	g_aeopid[471].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, rSFlying));
	g_aeopid[471].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, rSFlying));
	g_aeopid[471].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, rSFlying));
	g_aeopid[471].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[472].otyp = OTYP_Fask;
	g_aeopid[472].grfeopid = 4242;
	g_aeopid[472].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, bliprk));
	g_aeopid[472].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, bliprk));
	g_aeopid[472].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, bliprk));
	g_aeopid[472].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[473].otyp = OTYP_Lm;
	g_aeopid[473].grfeopid = 4242;
	g_aeopid[473].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, lmSw));
	g_aeopid[473].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, lmSw));
	g_aeopid[473].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, lmSw));
	g_aeopid[473].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[474].otyp = OTYP_Bool;
	g_aeopid[474].grfeopid = 4242;
	g_aeopid[474].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fRandomRoll));
	g_aeopid[474].optdat.ibSet = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fRandomRoll));
	g_aeopid[474].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitp) + offsetof(EMITP, emitblip) + offsetof(EMITBLIP, fRandomRoll));
	g_aeopid[474].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[475].otyp = OTYP_Bool;
	g_aeopid[475].grfeopid = 4226;
	g_aeopid[475].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, bullet) + offsetof(EMITX__bullet, fDamage));
	g_aeopid[475].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, bullet) + offsetof(EMITX__bullet, fDamage));
	g_aeopid[475].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[476].otyp = OTYP_Bool;
	g_aeopid[476].grfeopid = 4226;
	g_aeopid[476].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, bullet) + offsetof(EMITX__bullet, fRichochet));
	g_aeopid[476].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, bullet) + offsetof(EMITX__bullet, fRichochet));
	g_aeopid[476].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[477].otyp = OTYP_Sfxid;
	g_aeopid[477].grfeopid = 4242;
	g_aeopid[477].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SFX, sfxid));
	g_aeopid[477].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SFX, sfxid));
	g_aeopid[477].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SFX, sfxid));
	g_aeopid[477].optdat.pfnensure = (PFNENSURE)PsfxEnsureExpls;

	g_aeopid[478].otyp = OTYP_Float;
	g_aeopid[478].grfeopid = 4242;
	g_aeopid[478].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SFX, sStart));
	g_aeopid[478].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SFX, sStart));
	g_aeopid[478].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SFX, sStart));
	g_aeopid[478].optdat.pfnensure = (PFNENSURE)PsfxEnsureExpls;

	g_aeopid[479].otyp = OTYP_Float;
	g_aeopid[479].grfeopid = 4242;
	g_aeopid[479].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SFX, sFull));
	g_aeopid[479].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SFX, sFull));
	g_aeopid[479].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SFX, sFull));
	g_aeopid[479].optdat.pfnensure = (PFNENSURE)PsfxEnsureExpls;

	g_aeopid[480].otyp = OTYP_Float;
	g_aeopid[480].grfeopid = 4242;
	g_aeopid[480].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SFX, uVol));
	g_aeopid[480].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SFX, uVol));
	g_aeopid[480].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SFX, uVol));
	g_aeopid[480].optdat.pfnensure = (PFNENSURE)PsfxEnsureExpls;

	g_aeopid[481].otyp = OTYP_Void;
	g_aeopid[481].grfeopid = 1;
	g_aeopid[481].optdat.ibGet = 0;
	g_aeopid[481].optdat.ibSet = 0;
	g_aeopid[481].optdat.pvThunkFn = RefThunkExplsEXPLODE;

	g_aeopid[482].otyp = OTYP_Void;
	g_aeopid[482].grfeopid = 1;
	g_aeopid[482].optdat.ibGet = 0;
	g_aeopid[482].optdat.ibSet = 6;
	g_aeopid[482].optdat.pvThunkFn = RefThunkExplsEXPLODE_OVR;

	g_aeopid[483].otyp = OTYP_Oid;
	g_aeopid[483].grfeopid = 4226;
	g_aeopid[483].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, remit) + offsetof(EMITX__remit, oidExpls));
	g_aeopid[483].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, remit) + offsetof(EMITX__remit, oidExpls));
	g_aeopid[483].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[484].otyp = OTYP_Float;
	g_aeopid[484].grfeopid = 4226;
	g_aeopid[484].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, remit) + offsetof(EMITX__remit, svcParticle));
	g_aeopid[484].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, remit) + offsetof(EMITX__remit, svcParticle));
	g_aeopid[484].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[485].otyp = OTYP_Float;
	g_aeopid[485].grfeopid = 4226;
	g_aeopid[485].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, shrapnel) + offsetof(EMITX__shrapnel, sRadius));
	g_aeopid[485].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, shrapnel) + offsetof(EMITX__shrapnel, sRadius));
	g_aeopid[485].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[486].otyp = OTYP_Float;
	g_aeopid[486].grfeopid = 4226;
	g_aeopid[486].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, shrapnel) + offsetof(EMITX__shrapnel, elas));
	g_aeopid[486].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, shrapnel) + offsetof(EMITX__shrapnel, elas));
	g_aeopid[486].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[487].otyp = OTYP_Float;
	g_aeopid[487].grfeopid = 4226;
	g_aeopid[487].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, shrapnel) + offsetof(EMITX__shrapnel, mu));
	g_aeopid[487].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, shrapnel) + offsetof(EMITX__shrapnel, mu));
	g_aeopid[487].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[488].otyp = OTYP_Bool;
	g_aeopid[488].grfeopid = 4226;
	g_aeopid[488].optdat.ibGet = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, bullet) + offsetof(EMITX__bullet, fDamage));
	g_aeopid[488].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(EMITB, emitx) + offsetof(EMITB::EMITX, bullet) + offsetof(EMITX__bullet, fDamage));
	g_aeopid[488].optdat.pfnensure = (PFNENSURE)PemitbEnsureExplo;

	g_aeopid[489].otyp = OTYP_Void;
	g_aeopid[489].grfeopid = 1;
	g_aeopid[489].optdat.ibGet = 0;
	g_aeopid[489].optdat.ibSet = 0;
	g_aeopid[489].optdat.pvThunkFn = RefThunkExplsEXPLODE;

	g_aeopid[490].otyp = OTYP_Void;
	g_aeopid[490].grfeopid = 1;
	g_aeopid[490].optdat.ibGet = 0;
	g_aeopid[490].optdat.ibSet = 6;
	g_aeopid[490].optdat.pvThunkFn = RefThunkExplsEXPLODE_OVR;

	g_aeopid[491].otyp = OTYP_Seek;
	g_aeopid[491].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, lightk));
	g_aeopid[491].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetLightKind);
	g_aeopid[491].grfeopid = 290;
	g_aeopid[491].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightKind);

	g_aeopid[492].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightHighlightColor);
	g_aeopid[492].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(LIGHT, vecHighlight));
	g_aeopid[492].otyp = OTYP_Vector;
	g_aeopid[492].grfeopid = 162;
	g_aeopid[492].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, vecHighlight));

	g_aeopid[493].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightMidtoneStrength);
	g_aeopid[493].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(LIGHT, gMidtone));
	g_aeopid[493].otyp = OTYP_Float;
	g_aeopid[493].grfeopid = 162;
	g_aeopid[493].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, gMidtone));

	g_aeopid[494].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightShadowStrength);
	g_aeopid[494].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(LIGHT, gShadow));
	g_aeopid[494].otyp = OTYP_Float;
	g_aeopid[494].grfeopid = 162;
	g_aeopid[494].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, gShadow));

	g_aeopid[495].grfeopid = 162;
	g_aeopid[495].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightHighlightAngle);
	g_aeopid[495].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(LIGHT, degHighlight));
	g_aeopid[495].otyp = OTYP_Float;
	g_aeopid[495].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, degHighlight));

	g_aeopid[496].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightMidtoneAngle);
	g_aeopid[496].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(LIGHT, degMidtone));
	g_aeopid[496].otyp = OTYP_Float;
	g_aeopid[496].grfeopid = 162;
	g_aeopid[496].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, degMidtone));

	g_aeopid[497].otyp = OTYP_Float;
	g_aeopid[497].grfeopid = 162;
	g_aeopid[497].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, degShadow));
	g_aeopid[497].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightShadowAngle);
	g_aeopid[497].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(LIGHT, degShadow));

	g_aeopid[498].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, vecDirectionOrig));
	g_aeopid[498].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetLightDirection);
	g_aeopid[498].otyp = OTYP_Vector;
	g_aeopid[498].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightDirection);
	g_aeopid[498].grfeopid = 290;

	g_aeopid[499].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, fDynamic));
	g_aeopid[499].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightDynamic);
	g_aeopid[499].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetLightDynamic);
	g_aeopid[499].otyp = OTYP_Bool;
	g_aeopid[499].grfeopid = 290;

	g_aeopid[500].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, fDynamic));
	g_aeopid[500].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetLightDynamic);
	g_aeopid[500].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightDynamic);
	g_aeopid[500].grfeopid = 290;
	g_aeopid[500].otyp = OTYP_Bool;

	g_aeopid[501].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(LIGHT, fExcludeDynamicObjects));
	g_aeopid[501].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, fExcludeDynamicObjects));
	g_aeopid[501].optdat.ibSet = static_cast<std::intptr_t>(offsetof(LIGHT, fExcludeDynamicObjects));
	g_aeopid[501].grfeopid = 146;
	g_aeopid[501].otyp = OTYP_Bool;

	g_aeopid[502].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightFallOff);
	g_aeopid[502].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(LIGHT, lmFallOffS));
	g_aeopid[502].otyp = OTYP_Lm;
	g_aeopid[502].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, lmFallOffS));
	g_aeopid[502].grfeopid = 162;

	g_aeopid[503].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightConeAngle);
	g_aeopid[503].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(LIGHT, degCone));
	g_aeopid[503].otyp = OTYP_Float;
	g_aeopid[503].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, degCone));
	g_aeopid[503].grfeopid = 162;

	g_aeopid[504].otyp = OTYP_Float;
	g_aeopid[504].grfeopid = 162;
	g_aeopid[504].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(LIGHT, degHotSpot));
	g_aeopid[504].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightHotSpotAngle);
	g_aeopid[504].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, degHotSpot));

	g_aeopid[505].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(LIGHT, vecUpLocal));
	g_aeopid[505].otyp = OTYP_Vector;
	g_aeopid[505].grfeopid = 162;
	g_aeopid[505].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightFrustrumUp);
	g_aeopid[505].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIGHT, vecUpLocal));

	/*g_aeopid[506].otyp = OTYP_Float;
	g_aeopid[506].grfeopid = 292;
	g_aeopid[506].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetLightDiffuse_Old);
	g_aeopid[506].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightDiffuse_Old);
	g_aeopid[506].optdat.pvThunkFnUser = GetLightDiffuse_Old;

	g_aeopid[507].otyp = OTYP_Vector;
	g_aeopid[507].grfeopid = 292;
	g_aeopid[507].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetLightSaturate_Old);
	g_aeopid[507].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLightSaturate_Old);
	g_aeopid[507].optdat.pvThunkFnUser = GetLightSaturate_Old;*/

	g_aeopid[508].otyp = OTYP_Bool;
	g_aeopid[508].grfeopid = 0;

	g_aeopid[509].otyp = OTYP_Float;
	g_aeopid[509].grfeopid = 0;

	g_aeopid[510].otyp = OTYP_Float;
	g_aeopid[510].grfeopid = 0;

	g_aeopid[511].otyp = OTYP_Float;
	g_aeopid[511].grfeopid = 0;

	g_aeopid[512].otyp = OTYP_Float;
	g_aeopid[512].grfeopid = 0;

	g_aeopid[513].grfeopid = 128;
	g_aeopid[513].otyp = OTYP_Bool;
	g_aeopid[513].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SM, fDefault));

	g_aeopid[514].otyp = 0x2093;
	g_aeopid[514].grfeopid = 1;
	g_aeopid[514].optdat.ibGet = 0;
	g_aeopid[514].optdat.ibSet = 3;
	g_aeopid[514].optdat.pvThunkFn = RefThunkSmAPPLY;

	g_aeopid[515].otyp = OTYP_Alo;
	g_aeopid[515].grfeopid = 2;
	g_aeopid[515].optdat.ibGet = static_cast<int>(offsetof(SMA, paloRoot));

	g_aeopid[516].otyp = OTYP_Oid;
	g_aeopid[516].grfeopid = 36;
	g_aeopid[516].optdat.pfnget = reinterpret_cast<PFNRAW>(GetSmaGoal);
	g_aeopid[516].optdat.pfnset = reinterpret_cast<PFNRAW>(SetSmaGoal);

	g_aeopid[517].otyp = OTYP_Oid;
	g_aeopid[517].grfeopid = 4;
	g_aeopid[517].optdat.pfnget = reinterpret_cast<PFNRAW>(GetSmaCur);

	g_aeopid[518].otyp = OTYP_Oid;
	g_aeopid[518].grfeopid = 4;
	g_aeopid[518].optdat.pfnget = reinterpret_cast<PFNRAW>(GetSmaNext);

	g_aeopid[519].otyp = OTYP_Float;
	g_aeopid[519].grfeopid = 34;
	g_aeopid[519].optdat.ibGet = static_cast<int>(offsetof(SMA, svtLocal));
	g_aeopid[519].optdat.pfnset = reinterpret_cast<PFNRAW>(SetSmaSvt);

	g_aeopid[520].otyp = OTYP_Asegbl;
	g_aeopid[520].grfeopid = 2;
	g_aeopid[520].optdat.ibGet = static_cast<int>(offsetof(SMA, pasegaCur));

	g_aeopid[521].otyp = OTYP_Oid;
	g_aeopid[521].grfeopid = 32;
	g_aeopid[521].optdat.pfnset = reinterpret_cast<PFNRAW>(SeekSma);

	g_aeopid[522].otyp = OTYP_Void;
	g_aeopid[522].grfeopid = 1;
	g_aeopid[522].optdat.ibGet = 0;
	g_aeopid[522].optdat.ibSet = 0;
	g_aeopid[522].optdat.pvThunkFn = RefThunkSmaRETRACT;

	g_aeopid[523].otyp = OTYP_Exit;
	g_aeopid[523].grfeopid = 2;
	g_aeopid[523].optdat.ibGet = static_cast<int>(offsetof(SMA, psm));

	g_aeopid[524].otyp = OTYP_Bool;
	g_aeopid[524].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SW, fMusicEnabled));
	g_aeopid[524].grfeopid = 2176;
	g_aeopid[524].optdat.fDef = 1;

	g_aeopid[525].grfeopid = 2176;
	g_aeopid[525].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SW, uMusicVolume));
	g_aeopid[525].otyp = OTYP_Float;
	g_aeopid[525].optdat.fDef = 1065353216;

	g_aeopid[526].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SW, lsmDefault) + offsetof(LSM, uMidtone));
	g_aeopid[526].otyp = OTYP_Float;
	g_aeopid[526].grfeopid = 162;
	g_aeopid[526].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSwIllum);
	g_aeopid[526].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SW, lsmDefault) + offsetof(LSM, uMidtone));

	g_aeopid[527].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SW, lsmDefault) + offsetof(LSM, uShadow));
	g_aeopid[527].otyp = OTYP_Float;
	g_aeopid[527].grfeopid = 162;
	g_aeopid[527].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSwIllumShadow);
	g_aeopid[527].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SW, lsmDefault) + offsetof(LSM, uShadow));

	g_aeopid[528].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SW, rgbaSky));
	g_aeopid[528].otyp = OTYP_Rgba;
	g_aeopid[528].grfeopid = 128;

	g_aeopid[529].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSwCameraFov);
	g_aeopid[529].otyp = OTYP_Float;
	g_aeopid[529].grfeopid = 256;

	g_aeopid[530].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSwCameraNearClip);
	g_aeopid[530].otyp = OTYP_Float;
	g_aeopid[530].grfeopid = 256;

	g_aeopid[531].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSwCameraFarClip);
	g_aeopid[531].otyp = OTYP_Float;
	g_aeopid[531].grfeopid = 256;

	g_aeopid[532].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSwCameraNearFog);
	g_aeopid[532].otyp = OTYP_Float;
	g_aeopid[532].grfeopid = 256;

	g_aeopid[533].otyp = OTYP_Float;
	g_aeopid[533].grfeopid = 256;
	g_aeopid[533].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSwCameraFarFog);

	g_aeopid[534].otyp = OTYP_Float;
	g_aeopid[534].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSwCameraUFogMax);
	g_aeopid[534].grfeopid = 256;

	g_aeopid[535].otyp = OTYP_Rgba;
	g_aeopid[535].grfeopid = 256;
	g_aeopid[535].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSwCameraRgbaFog);

	g_aeopid[536].otyp = OTYP_Float;
	g_aeopid[536].grfeopid = 256;
	g_aeopid[536].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSwCameraMrdRatio);

	g_aeopid[537].otyp = OTYP_Float;
	g_aeopid[537].grfeopid = 2194;
	g_aeopid[537].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SW, tGoldEnable));
	g_aeopid[537].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SW, tGoldEnable));
	g_aeopid[537].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SW, tGoldEnable));
	g_aeopid[537].optdat.fDef = 1097859072;

	g_aeopid[538].otyp = OTYP_Float;
	g_aeopid[538].grfeopid = 2194;
	g_aeopid[538].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SW, dtCommentaryAudio));
	g_aeopid[538].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SW, dtCommentaryAudio));
	g_aeopid[538].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SW, dtCommentaryAudio));
	g_aeopid[538].optdat.fDef = 1127481344;

	g_aeopid[539].otyp = OTYP_Float;
	g_aeopid[539].grfeopid = 34;
	g_aeopid[539].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SW, gexcMenu));
	g_aeopid[539].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSwExcitement);

	g_aeopid[540].otyp = OTYP_Float;
	g_aeopid[540].grfeopid = 34;
	g_aeopid[540].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SW, rDarken));
	g_aeopid[540].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSwDarken);

	g_aeopid[541].grfeopid = 34;
	g_aeopid[541].otyp = OTYP_Float;
	g_aeopid[541].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SW, rDarkenSmooth));
	g_aeopid[541].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSwDarkenSmooth);

	g_aeopid[542].otyp = OTYP_Int;
	g_aeopid[542].grfeopid = 4242;
	g_aeopid[542].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DIFFICULTY, ccoinRichMin));
	g_aeopid[542].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DIFFICULTY, ccoinRichMin));
	g_aeopid[542].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DIFFICULTY, ccoinRichMin));
	g_aeopid[542].optdat.pfnensure = (PFNENSURE)PdifficultyEnsureSw;

	g_aeopid[543].grfeopid = 4242;
	g_aeopid[543].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DIFFICULTY, ccoinRichMax));
	g_aeopid[543].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DIFFICULTY, ccoinRichMax));
	g_aeopid[543].otyp = OTYP_Int;
	g_aeopid[543].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DIFFICULTY, ccoinRichMax));
	g_aeopid[543].optdat.pfnensure = (PFNENSURE)PdifficultyEnsureSw;

	g_aeopid[544].grfeopid = 4242;
	g_aeopid[544].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DIFFICULTY, ccoinPoorMin));
	g_aeopid[544].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DIFFICULTY, ccoinPoorMin));
	g_aeopid[544].otyp = OTYP_Int;
	g_aeopid[544].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DIFFICULTY, ccoinPoorMin));
	g_aeopid[544].optdat.pfnensure = (PFNENSURE)PdifficultyEnsureSw;

	g_aeopid[545].otyp = OTYP_Int;
	g_aeopid[545].grfeopid = 4242;
	g_aeopid[545].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DIFFICULTY, ccoinPoorMax));
	g_aeopid[545].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DIFFICULTY, ccoinPoorMax));
	g_aeopid[545].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DIFFICULTY, ccoinPoorMax));
	g_aeopid[545].optdat.pfnensure = (PFNENSURE)PdifficultyEnsureSw;

	g_aeopid[546].optdat.ibSet = 3;
	g_aeopid[546].grfeopid = 1;
	g_aeopid[546].otyp = OTYP_Void;
	g_aeopid[546].optdat.ibGet = 0;
	g_aeopid[546].optdat.pvThunkFn = RefThunkSwSpawnRandomCoins;

	g_aeopid[547].optdat.pvThunkFnUser = (PVTHUNKFN)GetSwPlayerSuck;
	g_aeopid[547].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetDifficultySuck);
	g_aeopid[547].grfeopid = 36;
	g_aeopid[547].otyp = OTYP_Float;

	g_aeopid[548].otyp = OTYP_Float;
	g_aeopid[548].grfeopid = 36;
	g_aeopid[548].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSuckRaw);
	g_aeopid[548].optdat.pvThunkFnUser = (PVTHUNKFN)GetuSuckRaw;

	g_aeopid[549].otyp = OTYP_Sfxid;
	g_aeopid[549].grfeopid = 288;
	g_aeopid[549].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSwIntermittentSound);
	g_aeopid[549].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSwIntermittentSound);

	g_aeopid[550].otyp = OTYP_Lm;
	g_aeopid[550].grfeopid = 288;
	g_aeopid[550].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSwIntermittentVolPct);
	g_aeopid[550].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSwIntermittentVolPct);

	g_aeopid[551].otyp = OTYP_Lm;
	g_aeopid[551].grfeopid = 288;
	g_aeopid[551].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSwIntermittentRepeat);
	g_aeopid[551].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSwIntermittentRepeat);

	g_aeopid[552].otyp = OTYP_Void;
	g_aeopid[552].optdat.pvThunkFnUser = RefThunkSwSetDefaultReverb;
	g_aeopid[552].grfeopid = 1024;
	g_aeopid[552].optdat.ibSet = 2;
	g_aeopid[552].optdat.pvThunkFn = nullptr;

	g_aeopid[553].grfeopid = 1;
	g_aeopid[553].otyp = OTYP_Void;
	g_aeopid[553].optdat.ibSet = 2;
	g_aeopid[553].optdat.pvThunkFn = RefThunkSwPUSH_REVERB;
	g_aeopid[553].optdat.ibGet = 0;

	g_aeopid[554].otyp = OTYP_Void;
	g_aeopid[554].optdat.pvThunkFn = RefThunkSwPOP_REVERB;
	g_aeopid[554].grfeopid = 1;
	g_aeopid[554].optdat.ibGet = 0;
	g_aeopid[554].optdat.ibSet = 0;

	g_aeopid[555].otyp = OTYP_Bool;
	g_aeopid[555].grfeopid = 1;
	g_aeopid[555].optdat.ibGet = 0;
	g_aeopid[555].optdat.pvThunkFn = RefThunkSwLEVEL_VISITED;
	g_aeopid[555].optdat.ibSet = 1;

	g_aeopid[556].optdat.pvThunkFn = RefThunkSwLEVEL_PRIMARY;
	g_aeopid[556].otyp = OTYP_Bool;
	g_aeopid[556].grfeopid = 1;
	g_aeopid[556].optdat.ibSet = 1;
	g_aeopid[556].optdat.ibGet = 0;

	g_aeopid[557].optdat.pvThunkFn = RefThunkSwLEVEL_SECONDARY;
	g_aeopid[557].grfeopid = 1;
	g_aeopid[557].optdat.ibSet = 1;
	g_aeopid[557].otyp = OTYP_Bool;
	g_aeopid[557].optdat.ibGet = 0;

	g_aeopid[558].optdat.pvThunkFn = RefThunkSwLEVEL_TERTIARY;
	g_aeopid[558].grfeopid = 1;
	g_aeopid[558].optdat.ibSet = 1;
	g_aeopid[558].otyp = OTYP_Bool;
	g_aeopid[558].optdat.ibGet = 0;

	g_aeopid[559].optdat.pvThunkFn = RefThunkSwLEVEL_KEY_COLLECTED;
	g_aeopid[559].grfeopid = 1;
	g_aeopid[559].optdat.ibSet = 1;
	g_aeopid[559].otyp = OTYP_Bool;
	g_aeopid[559].optdat.ibGet = 0;

	g_aeopid[560].optdat.pvThunkFn = RefThunkSwSECONDARY_TASK_AVAILABLE;
	g_aeopid[560].grfeopid = 1;
	g_aeopid[560].optdat.ibSet = 1;
	g_aeopid[560].otyp = OTYP_Bool;
	g_aeopid[560].optdat.ibGet = 0;

	g_aeopid[561].optdat.pvThunkFn = RefThunkSwCOMMENTARY_UNLOCKED_FOR_LEVEL;
	g_aeopid[561].grfeopid = 1;
	g_aeopid[561].optdat.ibSet = 1;
	g_aeopid[561].otyp = OTYP_Bool;
	g_aeopid[561].optdat.ibGet = 0;

	g_aeopid[562].otyp = OTYP_Void;
	g_aeopid[562].grfeopid = 1;
	g_aeopid[562].optdat.ibGet = 0;
	g_aeopid[562].optdat.pvThunkFn = RefThunkSwSET_LEVEL_DATA_VALUE;
	g_aeopid[562].optdat.ibSet = 3;

	g_aeopid[563].optdat.pvThunkFn = RefThunkSwGET_LEVEL_DATA_VALUE;
	g_aeopid[563].otyp = OTYP_Int;
	g_aeopid[563].grfeopid = 1;
	g_aeopid[563].optdat.ibGet = 0;
	g_aeopid[563].optdat.ibSet = 2;

	g_aeopid[564].otyp = OTYP_Bool;
	g_aeopid[564].grfeopid = 1;
	g_aeopid[564].optdat.pvThunkFn = RefThunkSwWORLD_VISITED;
	g_aeopid[564].optdat.ibGet = 0;
	g_aeopid[564].optdat.ibSet = 1;

	g_aeopid[565].otyp = OTYP_Bool;
	g_aeopid[565].grfeopid = 1;
	g_aeopid[565].optdat.pvThunkFn = RefThunkSwWORLD_COMPLETE;
	g_aeopid[565].optdat.ibGet = 0;
	g_aeopid[565].optdat.ibSet = 1;

	g_aeopid[566].otyp = OTYP_Int;
	g_aeopid[566].grfeopid = 1;
	g_aeopid[566].optdat.pvThunkFn = RefThunkSwRECALCULATE_PERCENT_COMPLETION;
	g_aeopid[566].optdat.ibGet = 0;
	g_aeopid[566].optdat.ibSet = 0;

	g_aeopid[567].optdat.pvThunkFnUser = (PVTHUNKFN)IsSwVagPlaying;
	g_aeopid[567].otyp = OTYP_Bool;
	g_aeopid[567].grfeopid = 4;

	g_aeopid[568].otyp = OTYP_Void;
	g_aeopid[568].optdat.pvThunkFn = RefThunkSwCANCEL_DIALOG;
	g_aeopid[568].grfeopid = 1;
	g_aeopid[568].optdat.ibGet = 0;
	g_aeopid[568].optdat.ibSet = 0;

	g_aeopid[569].otyp = OTYP_Void;
	g_aeopid[569].grfeopid = 1;
	g_aeopid[569].optdat.ibGet = 0;
	g_aeopid[569].optdat.pvThunkFn = RefThunkSwSET_SMA_PROMPT;
	g_aeopid[569].optdat.ibSet = 2;

	g_aeopid[570].otyp = OTYP_Void;
	g_aeopid[570].optdat.pvThunkFn = RefThunkSwACQUIRE_LETTERBOX;
	g_aeopid[570].grfeopid = 1;
	g_aeopid[570].optdat.ibSet = 0;
	g_aeopid[570].optdat.ibGet = 0;

	g_aeopid[571].optdat.pvThunkFn = RefThunkSwRELEASE_LETTERBOX;
	g_aeopid[571].otyp = OTYP_Void;
	g_aeopid[571].grfeopid = 1;
	g_aeopid[571].optdat.ibSet = 0;
	g_aeopid[571].optdat.ibGet = 0;

	g_aeopid[572].grfeopid = 146;
	g_aeopid[572].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SW, fDprizeEnabled));
	g_aeopid[572].otyp = OTYP_Bool;
	g_aeopid[572].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SW, fDprizeEnabled));
	g_aeopid[572].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SW, fDprizeEnabled));

	g_aeopid[573].optdat.pvThunkFn = RefThunkSwSET_EXCITEMENT;
	g_aeopid[573].otyp = OTYP_Int;
	g_aeopid[573].grfeopid = 1;
	g_aeopid[573].optdat.ibSet = 1;
	g_aeopid[573].optdat.ibGet = 0;

	g_aeopid[574].optdat.pvThunkFn = RefThunkSwUNSET_EXCITEMENT;
	g_aeopid[574].otyp = OTYP_Void;
	g_aeopid[574].grfeopid = 1;
	g_aeopid[574].optdat.ibSet = 1;
	g_aeopid[574].optdat.ibGet = 0;

	g_aeopid[575].optdat.pvThunkFn = RefThunkSwSET_MUSIC_REGISTER;
	g_aeopid[575].otyp = OTYP_Void;
	g_aeopid[575].grfeopid = 1;
	g_aeopid[575].optdat.ibSet = 2;
	g_aeopid[575].optdat.ibGet = 0;

	g_aeopid[576].otyp = OTYP_Int;
	g_aeopid[576].grfeopid = 1;
	g_aeopid[576].optdat.ibGet = 0;
	g_aeopid[576].optdat.pvThunkFn = RefThunkSwRESPONSE_TEXT;
	g_aeopid[576].optdat.ibSet = 1;

	g_aeopid[577].optdat.pvThunkFn = RefThunkSwREFRESH_MIDI_REGISTER;
	g_aeopid[577].otyp = OTYP_Void;
	g_aeopid[577].grfeopid = 1;
	g_aeopid[577].optdat.ibGet = 0;
	g_aeopid[577].optdat.ibSet = 2;

	g_aeopid[578].grfeopid = 1;
	g_aeopid[578].otyp = OTYP_Void;
	g_aeopid[578].optdat.ibGet = 0;
	g_aeopid[578].optdat.pvThunkFn = RefThunkSwHANDS_OFF;
	g_aeopid[578].optdat.ibSet = 0;

	g_aeopid[579].grfeopid = 1;
	g_aeopid[579].otyp = OTYP_Void;
	g_aeopid[579].optdat.ibGet = 0;
	g_aeopid[579].optdat.pvThunkFn = RefThunkSwHANDS_ON;
	g_aeopid[579].optdat.ibSet = 0;

	g_aeopid[580].optdat.pvThunkFnUser = RefThunkSwLOCK_VAULT;
	g_aeopid[580].grfeopid = 1025;
	g_aeopid[580].otyp = OTYP_Void;
	g_aeopid[580].optdat.ibSet = 1;
	g_aeopid[580].optdat.pvThunkFn = RefThunkSwLOCK_VAULT;

	g_aeopid[581].grfeopid = 1025;
	g_aeopid[581].optdat.pvThunkFnUser = RefThunkSwUNLOCK_VAULT;
	g_aeopid[581].otyp = OTYP_Void;
	g_aeopid[581].optdat.ibSet = 1;
	g_aeopid[581].optdat.pvThunkFn = RefThunkSwUNLOCK_VAULT;

	g_aeopid[582].optdat.pfnget = (PFNRAW)GetCharmCount;
	g_aeopid[582].grfeopid = 4;
	g_aeopid[582].otyp = OTYP_Int;

	g_aeopid[583].optdat.pfnget = (PFNRAW)GetLifeCount;
	g_aeopid[583].grfeopid = 4;
	g_aeopid[583].otyp = OTYP_Int;

	g_aeopid[584].otyp = OTYP_Vector;
	g_aeopid[584].grfeopid = 34;
	g_aeopid[584].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CM, pos));
	g_aeopid[584].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetCmPos);

	g_aeopid[585].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetCmMat);
	g_aeopid[585].otyp = OTYP_Matrix;
	g_aeopid[585].grfeopid = 34;
	g_aeopid[585].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CM, mat));

	g_aeopid[586].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetCmFov);
	g_aeopid[586].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CM, radFOV));
	g_aeopid[586].otyp = OTYP_Float;
	g_aeopid[586].grfeopid = 34;

	g_aeopid[587].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CM, radFOVTarget));
	g_aeopid[587].otyp = OTYP_Float;
	g_aeopid[587].grfeopid = 18;
	g_aeopid[587].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CM, radFOVTarget));

	g_aeopid[588].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CM, smpaRadFOV));
	g_aeopid[588].otyp = OTYP_Smpa;
	g_aeopid[588].grfeopid = 18;
	g_aeopid[588].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CM, smpaRadFOV));

	g_aeopid[589].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetCmNearClip);
	g_aeopid[589].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CM, sNearClip));
	g_aeopid[589].otyp = OTYP_Float;
	g_aeopid[589].grfeopid = 34;

	g_aeopid[590].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetCmFarClip);
	g_aeopid[590].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CM, sFarClip));
	g_aeopid[590].otyp = OTYP_Float;
	g_aeopid[590].grfeopid = 34;

	g_aeopid[591].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetCmNearFog);
	g_aeopid[591].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CM, sNearFog));
	g_aeopid[591].otyp = OTYP_Float;
	g_aeopid[591].grfeopid = 34;

	g_aeopid[592].otyp = OTYP_Float;
	g_aeopid[592].grfeopid = 34;
	g_aeopid[592].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CM, sFarFog));
	g_aeopid[592].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetCmFarFog);

	g_aeopid[593].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CM, uFogMax));
	g_aeopid[593].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetCmUFogMax);
	g_aeopid[593].otyp = OTYP_Float;
	g_aeopid[593].grfeopid = 34;

	g_aeopid[594].grfeopid = 34;
	g_aeopid[594].otyp = OTYP_Float;
	g_aeopid[594].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CM, rMRD));
	g_aeopid[594].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetCmMrdRatio);

	g_aeopid[595].optdat.pvThunkFn = RefThunkCmPUSH_LOOK_KIND;
	g_aeopid[595].otyp = OTYP_Void;
	g_aeopid[595].grfeopid = 1;
	g_aeopid[595].optdat.ibGet = 0;
	g_aeopid[595].optdat.ibSet = 1;

	g_aeopid[596].optdat.pvThunkFn = RefThunkCmPOP_LOOK_KIND;
	g_aeopid[596].otyp = OTYP_Wek;
	g_aeopid[596].grfeopid = 1;
	g_aeopid[596].optdat.ibGet = 0;
	g_aeopid[596].optdat.ibSet = 0;

	g_aeopid[597].otyp = OTYP_Wek;
	g_aeopid[597].optdat.pvThunkFn = RefThunkCmLOOK_KIND;
	g_aeopid[597].grfeopid = 1;
	g_aeopid[597].optdat.ibGet = 0;
	g_aeopid[597].optdat.ibSet = 0;

	g_aeopid[598].optdat.pvThunkFn = RefThunkCmSET_SNIPER_FOCUS;
	g_aeopid[598].otyp = OTYP_Void;
	g_aeopid[598].grfeopid = 1;
	g_aeopid[598].optdat.ibGet = 0;
	g_aeopid[598].optdat.ibSet = 3;

	g_aeopid[599].optdat.pvThunkFn = RefThunkCmJOLT;
	g_aeopid[599].otyp = OTYP_Void;
	g_aeopid[599].grfeopid = 1;
	g_aeopid[599].optdat.ibGet = 0;
	g_aeopid[599].optdat.ibSet = 0;

	g_aeopid[600].otyp = OTYP_Void;
	g_aeopid[600].grfeopid = 1;
	g_aeopid[600].optdat.pvThunkFn = RefThunkCmCUT;
	g_aeopid[600].optdat.ibGet = 0;
	g_aeopid[600].optdat.ibSet = 0;

	g_aeopid[601].otyp = OTYP_Bool;
	g_aeopid[601].grfeopid = 1;
	g_aeopid[601].optdat.ibSet = 3;
	g_aeopid[601].optdat.pvThunkFn = RefThunkCmVISIBLE_SPHERE;
	g_aeopid[601].optdat.ibGet = 0;

	g_aeopid[602].otyp = OTYP_Oid;
	g_aeopid[602].grfeopid = 128;
	g_aeopid[602].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CAMERA, oidTarget));

	g_aeopid[603].otyp = OTYP_Vector;
	g_aeopid[603].grfeopid = 128;
	g_aeopid[603].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CAMERA, posEye));

	g_aeopid[604].otyp = OTYP_Vector;
	g_aeopid[604].grfeopid = 128;
	g_aeopid[604].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CAMERA, vecView));

	g_aeopid[605].otyp = OTYP_Vector;
	g_aeopid[605].grfeopid = 128;
	g_aeopid[605].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CAMERA, vecUp));

	g_aeopid[606].otyp = OTYP_Void;
	g_aeopid[606].optdat.pvThunkFn = RefThunkCameraENABLE;
	g_aeopid[606].grfeopid = 1;
	g_aeopid[606].optdat.ibGet = 0;
	g_aeopid[606].optdat.ibSet = 0;

	g_aeopid[607].otyp = OTYP_Void;
	g_aeopid[607].optdat.pvThunkFn = RefThunkCameraDISABLE;
	g_aeopid[607].grfeopid = 1;
	g_aeopid[607].optdat.ibGet = 0;
	g_aeopid[607].optdat.ibSet = 0;

	g_aeopid[608].otyp = OTYP_Bool;
	g_aeopid[608].grfeopid = 146;
	g_aeopid[608].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SKY, fFollowZ));
	g_aeopid[608].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SKY, fFollowZ));
	g_aeopid[608].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SKY, fFollowZ));

	g_aeopid[609].otyp = OTYP_Fapl;
	g_aeopid[609].grfeopid = 146;
	g_aeopid[609].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TZP, zpd) + offsetof(ZPD, zpk));
	g_aeopid[609].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TZP, zpd) + offsetof(ZPD, zpk));
	g_aeopid[609].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TZP, zpd) + offsetof(ZPD, zpk));

	g_aeopid[610].otyp = OTYP_Oid;
	g_aeopid[610].grfeopid = 4352;
	g_aeopid[610].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddZpdZapObject);
	g_aeopid[610].optdat.pfnensure = (PFNENSURE)PzpdEnsureTzp;

	g_aeopid[611].otyp = OTYP_Lo;
	g_aeopid[611].grfeopid = 4128;
	g_aeopid[611].optdat.ibSet = reinterpret_cast<std::intptr_t>(&AddZpdZapLo);
	g_aeopid[611].optdat.pfnensure = (PFNENSURE)PzpdEnsureTzp;

	g_aeopid[612].otyp = OTYP_Lo;
	g_aeopid[612].grfeopid = 4128;
	g_aeopid[612].optdat.ibSet = reinterpret_cast<std::intptr_t>(&RemoveZpdZapLo);
	g_aeopid[612].optdat.pfnensure = (PFNENSURE)PzpdEnsureTzp;

	g_aeopid[613].otyp = OTYP_Void;
	g_aeopid[613].grfeopid = 1;
	g_aeopid[613].optdat.ibGet = 0;
	g_aeopid[613].optdat.pvThunkFn = RefThunkTzpRESET_TZP_THROW_COUNT;
	g_aeopid[613].optdat.ibSet = 0;

	g_aeopid[614].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TZP, zpd) + offsetof(ZPD, dzThrow));
	g_aeopid[614].otyp = OTYP_Float;
	g_aeopid[614].grfeopid = 146;
	g_aeopid[614].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TZP, zpd) + offsetof(ZPD, dzThrow));
	g_aeopid[614].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TZP, zpd) + offsetof(ZPD, dzThrow));

	g_aeopid[615].otyp = OTYP_Bool;
	g_aeopid[615].grfeopid = 146;
	g_aeopid[615].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TZP, zpd) + offsetof(ZPD, fThrowCut));
	g_aeopid[615].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TZP, zpd) + offsetof(ZPD, fThrowCut));
	g_aeopid[615].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TZP, zpd) + offsetof(ZPD, fThrowCut));

	g_aeopid[616].otyp = OTYP_Icek;
	g_aeopid[616].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TARGET, grftak));
	g_aeopid[616].grfeopid = 146;
	g_aeopid[616].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TARGET, grftak));
	g_aeopid[616].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TARGET, grftak));

	g_aeopid[617].grfeopid = 146;
	g_aeopid[617].otyp = OTYP_Float;
	g_aeopid[617].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TARGET, sRadiusTarget));
	g_aeopid[617].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TARGET, sRadiusTarget));
	g_aeopid[617].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TARGET, sRadiusTarget));

	g_aeopid[618].grfeopid = 146;
	g_aeopid[618].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TARGET, fHitTest));
	g_aeopid[618].otyp = OTYP_Bool;
	g_aeopid[618].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TARGET, fHitTest));
	g_aeopid[618].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TARGET, fHitTest));

	g_aeopid[619].grfeopid = 256;
	g_aeopid[619].otyp = OTYP_Oid;
	g_aeopid[619].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddChkpntVol);

	g_aeopid[620].grfeopid = 256;
	g_aeopid[620].otyp = OTYP_Oid;
	g_aeopid[620].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddChkpntAseg);

	g_aeopid[621].grfeopid = 256;
	g_aeopid[621].otyp = OTYP_Oid;
	g_aeopid[621].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetChkpntWarp);

	g_aeopid[622].grfeopid = 1;
	g_aeopid[622].otyp = OTYP_Void;
	g_aeopid[622].optdat.ibGet = 0;
	g_aeopid[622].optdat.pvThunkFn = RefThunkClueBREAK;
	g_aeopid[622].optdat.ibSet = 0;

	g_aeopid[623].grfeopid = 146;
	g_aeopid[623].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CLUE, dtFrameMax));
	g_aeopid[623].otyp = OTYP_Float;
	g_aeopid[623].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CLUE, dtFrameMax));
	g_aeopid[623].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CLUE, dtFrameMax));

	g_aeopid[624].grfeopid = 4;
	g_aeopid[624].optdat.pvThunkFnUser = (PVTHUNKFN)GetClueCollected;
	g_aeopid[624].otyp = OTYP_Bool;

	g_aeopid[625].grfeopid = 290;
	g_aeopid[625].otyp = OTYP_Float;
	g_aeopid[625].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetCnvoBeltSpeed);
	g_aeopid[625].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CNVO, svBelt));
	g_aeopid[625].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetCnvoBeltSpeed);

	g_aeopid[626].otyp = OTYP_Float;
	g_aeopid[626].grfeopid = 2194;
	g_aeopid[626].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HPNT, sFlattenRadius));
	g_aeopid[626].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HPNT, sFlattenRadius));
	g_aeopid[626].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HPNT, sFlattenRadius));
	g_aeopid[626].optdat.fDef = 1112014848;

	g_aeopid[627].otyp = OTYP_Cmk;
	g_aeopid[627].grfeopid = 2194;
	g_aeopid[627].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HPNT, jthk));
	g_aeopid[627].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HPNT, jthk));
	g_aeopid[627].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HPNT, jthk));
	g_aeopid[627].optdat.fDef = 0;

	g_aeopid[628].grfeopid = 146;
	g_aeopid[628].otyp = OTYP_Bool;
	g_aeopid[628].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HPNT, fTunnel));
	g_aeopid[628].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HPNT, fTunnel));
	g_aeopid[628].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HPNT, fTunnel));

	g_aeopid[629].otyp = OTYP_Oid;
	g_aeopid[629].grfeopid = 128;
	g_aeopid[629].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HPNT, oidTnHide));

	g_aeopid[630].grfeopid = 146;
	g_aeopid[630].otyp = OTYP_Bool;
	g_aeopid[630].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HPNT, fDetect));
	g_aeopid[630].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HPNT, fDetect));
	g_aeopid[630].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HPNT, fDetect));

	g_aeopid[631].otyp = OTYP_Float;
	g_aeopid[631].grfeopid = 146;
	g_aeopid[631].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HPNT, dzJumpTargetMax));
	g_aeopid[631].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HPNT, dzJumpTargetMax));
	g_aeopid[631].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HPNT, dzJumpTargetMax));

	g_aeopid[632].grfeopid = 146;
	g_aeopid[632].otyp = OTYP_Bool;
	g_aeopid[632].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HPNT, unkInt));
	g_aeopid[632].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HPNT, unkInt));
	g_aeopid[632].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HPNT, unkInt));

	g_aeopid[633].otyp = OTYP_Cmk;
	g_aeopid[633].grfeopid = 2194;
	g_aeopid[633].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HSHAPE, jthk));
	g_aeopid[633].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HSHAPE, jthk));
	g_aeopid[633].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HSHAPE, jthk));
	g_aeopid[633].optdat.fDef = 0;

	g_aeopid[634].grfeopid = 146;
	g_aeopid[634].otyp = OTYP_Bool;
	g_aeopid[634].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HSHAPE, fTunnel));
	g_aeopid[634].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HSHAPE, fTunnel));
	g_aeopid[634].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HSHAPE, fTunnel));

	g_aeopid[635].otyp = OTYP_Oid;
	g_aeopid[635].grfeopid = 128;
	g_aeopid[635].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HSHAPE, oidTnHide));

	g_aeopid[636].otyp = OTYP_Rwmtk;
	g_aeopid[636].grfeopid = 146;
	g_aeopid[636].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HSHAPE, grfhp));
	g_aeopid[636].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HSHAPE, grfhp));
	g_aeopid[636].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HSHAPE, grfhp));

	g_aeopid[637].grfeopid = 146;
	g_aeopid[637].otyp = OTYP_Bool;
	g_aeopid[637].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HSHAPE, fDetect));
	g_aeopid[637].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HSHAPE, fDetect));
	g_aeopid[637].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HSHAPE, fDetect));

	g_aeopid[638].otyp = OTYP_Float;
	g_aeopid[638].grfeopid = 146;
	g_aeopid[638].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HSHAPE, dzHideMax));
	g_aeopid[638].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HSHAPE, dzHideMax));
	g_aeopid[638].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HSHAPE, dzHideMax));

	g_aeopid[639].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HSHAPE, dzMax));
	g_aeopid[639].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HSHAPE, dzMax));
	g_aeopid[639].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HSHAPE, dzMax));
	g_aeopid[639].otyp = OTYP_Bool;
	g_aeopid[639].grfeopid = 146;

	g_aeopid[640].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HBSK, sFlattenRadius));
	g_aeopid[640].otyp = OTYP_Float;
	g_aeopid[640].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HBSK, sFlattenRadius));
	g_aeopid[640].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HBSK, sFlattenRadius));
	g_aeopid[640].grfeopid = 2194;
	g_aeopid[640].optdat.fDef = 1112014848;

	g_aeopid[641].otyp = OTYP_Int;
	g_aeopid[641].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HBSK, cMaxDartsStuck));
	g_aeopid[641].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HBSK, cMaxDartsStuck));
	g_aeopid[641].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HBSK, cMaxDartsStuck));
	g_aeopid[641].grfeopid = 2194;
	g_aeopid[641].optdat.fDef = 10;

	g_aeopid[642].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(HBSK, dzJumpMax));
	g_aeopid[642].otyp = OTYP_Float;
	g_aeopid[642].optdat.ibGet = static_cast<std::intptr_t>(offsetof(HBSK, dzJumpMax));
	g_aeopid[642].optdat.ibSet = static_cast<std::intptr_t>(offsetof(HBSK, dzJumpMax));
	g_aeopid[642].grfeopid = 146;

	g_aeopid[643].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PIPE, vecFace));
	g_aeopid[643].otyp = OTYP_Vector;
	g_aeopid[643].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PIPE, vecFace));
	g_aeopid[643].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PIPE, vecFace));
	g_aeopid[643].grfeopid = 146;

	g_aeopid[644].otyp = OTYP_Sfxid;
	g_aeopid[644].grfeopid = 146;
	g_aeopid[644].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PIPE, sfxidGrab));
	g_aeopid[644].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PIPE, sfxidGrab));
	g_aeopid[644].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PIPE, sfxidGrab));

	g_aeopid[645].otyp = OTYP_Sfxid;
	g_aeopid[645].grfeopid = 146;
	g_aeopid[645].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PIPE, sfxidRelease));
	g_aeopid[645].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PIPE, sfxidRelease));
	g_aeopid[645].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PIPE, sfxidRelease));

	g_aeopid[646].otyp = OTYP_Lootek;
	g_aeopid[646].grfeopid = 146;
	g_aeopid[646].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PIPE, unkInt0));
	g_aeopid[646].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PIPE, unkInt0));
	g_aeopid[646].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PIPE, unkInt0));

	g_aeopid[647].otyp = OTYP_Void;
	g_aeopid[647].grfeopid = 0;

	g_aeopid[648].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PIPE, unkInt1));
	g_aeopid[648].otyp = OTYP_Bool;
	g_aeopid[648].grfeopid = 146;
	g_aeopid[648].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PIPE, unkInt1));
	g_aeopid[648].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PIPE, unkInt1));

	g_aeopid[649].otyp = OTYP_Oid;
	g_aeopid[649].grfeopid = 128;
	g_aeopid[649].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PIPE, oidRope));

	g_aeopid[650].otyp = OTYP_Rope;
	g_aeopid[650].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PIPE, pshape));
	g_aeopid[650].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PIPE, pshape));
	g_aeopid[650].grfeopid = 18;

	g_aeopid[651].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PIPE, dzJumpTargetMax));
	g_aeopid[651].otyp = OTYP_Float;
	g_aeopid[651].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PIPE, dzJumpTargetMax));
	g_aeopid[651].optdat.ibSet = static_cast<std::intptr_t>(offsetof(PIPE, dzJumpTargetMax));
	g_aeopid[651].grfeopid = 146;

	g_aeopid[652].otyp = OTYP_Oid;
	g_aeopid[652].grfeopid = 162;
	g_aeopid[652].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetStepguardPathzone);
	g_aeopid[652].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, oidPathzone));
	g_aeopid[652].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, oidPathzone));

	g_aeopid[653].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, lmAttackS));
	g_aeopid[653].grfeopid = 146;
	g_aeopid[653].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, lmAttackS));
	g_aeopid[653].otyp = OTYP_Lm;
	g_aeopid[653].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, lmAttackS));

	g_aeopid[654].grfeopid = 146;
	g_aeopid[654].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, lmAttackT));
	g_aeopid[654].otyp = OTYP_Lm;
	g_aeopid[654].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, lmAttackT));
	g_aeopid[654].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, lmAttackT));

	g_aeopid[655].grfeopid = 274;
	g_aeopid[655].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, radAttackMax));
	g_aeopid[655].otyp = OTYP_Float;
	g_aeopid[655].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, radAttackMax));
	g_aeopid[655].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetStepguardAttackAngleMax);

	g_aeopid[656].grfeopid = 1024;
	g_aeopid[656].optdat.pvThunkFnUser = RefThunkStepguardUSE_ANIMATION;
	g_aeopid[656].optdat.ibSet = 2;
	g_aeopid[656].otyp = OTYP_Void;
	g_aeopid[656].optdat.pvThunkFn = nullptr;

	g_aeopid[657].grfeopid = 1;
	g_aeopid[657].otyp = OTYP_Void;
	g_aeopid[657].optdat.pvThunkFn = RefThunkStepguardUSE_ANIMATION_IMMEDIATE;
	g_aeopid[657].optdat.ibGet = 0;
	g_aeopid[657].optdat.ibSet = 2;

	g_aeopid[658].grfeopid = 1;
	g_aeopid[658].otyp = OTYP_Explo;
	g_aeopid[658].optdat.pvThunkFn = RefThunkStepguardGET_ANIMATION_IMMEDIATE;
	g_aeopid[658].optdat.ibGet = 0;
	g_aeopid[658].optdat.ibSet = 1;

	g_aeopid[659].otyp = OTYP_Float;
	g_aeopid[659].grfeopid = 146;
	g_aeopid[659].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, dtFade));
	g_aeopid[659].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, dtFade));
	g_aeopid[659].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, dtFade));

	g_aeopid[660].otyp = OTYP_Void;
	g_aeopid[660].grfeopid = 1024;
	g_aeopid[660].optdat.pvThunkFnUser = RefThunkStepguardMATCH_ANIMATION_PHASE;
	g_aeopid[660].optdat.ibSet = 4;
	g_aeopid[660].optdat.pvThunkFn = nullptr;

	g_aeopid[661].optdat.ibSet = 2;
	g_aeopid[661].otyp = OTYP_Void;
	g_aeopid[661].grfeopid = 1024;
	g_aeopid[661].optdat.pvThunkFnUser = RefThunkStepguardADD_ATTACK_EFFECT;
	g_aeopid[661].optdat.pvThunkFn = nullptr;

	g_aeopid[662].otyp = OTYP_Void;
	g_aeopid[662].grfeopid = 1024;
	g_aeopid[662].optdat.pvThunkFnUser = RefThunkStepguardUSE_DEATH_ANIMATION;
	g_aeopid[662].optdat.ibSet = 2;
	g_aeopid[662].optdat.pvThunkFn = nullptr;

	g_aeopid[663].otyp = OTYP_Void;
	g_aeopid[663].grfeopid = 1;
	g_aeopid[663].optdat.ibSet = 2;
	g_aeopid[663].optdat.pvThunkFnUser = nullptr;
	g_aeopid[663].optdat.pvThunkFn = RefThunkStepguardSET_STATE_EXTERNAL;

	g_aeopid[664].otyp = OTYP_Void;
	g_aeopid[664].grfeopid = 1024;
	g_aeopid[664].optdat.pvThunkFnUser = RefThunkStepguardUSE_PHYS;
	g_aeopid[664].optdat.ibSet = 2;
	g_aeopid[664].optdat.pvThunkFn = nullptr;

	g_aeopid[665].otyp = OTYP_Int;
	g_aeopid[665].grfeopid = 146;
	g_aeopid[665].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, cStun));
	g_aeopid[665].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, cStun));
	g_aeopid[665].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, cStun));

	g_aeopid[666].grfeopid = 146;
	g_aeopid[666].otyp = OTYP_Bool;
	g_aeopid[666].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, fTumbleStun));
	g_aeopid[666].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fTumbleStun));
	g_aeopid[666].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fTumbleStun));

	g_aeopid[667].otyp = OTYP_Void;
	g_aeopid[667].grfeopid = 1;
	g_aeopid[667].optdat.ibGet = 0;
	g_aeopid[667].optdat.ibSet = 0;
	g_aeopid[667].optdat.pvThunkFn = RefThunkStepguardJUMP;

	g_aeopid[668].otyp = OTYP_Lm;
	g_aeopid[668].grfeopid = 146;
	g_aeopid[668].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, lmDtFreefall));
	g_aeopid[668].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, lmDtFreefall));
	g_aeopid[668].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, lmDtFreefall));

	g_aeopid[669].grfeopid = 146;
	g_aeopid[669].otyp = OTYP_Bool;
	g_aeopid[669].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fLockLook));
	g_aeopid[669].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fLockLook));
	g_aeopid[669].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, fLockLook));

	g_aeopid[670].otyp = OTYP_Oid;
	g_aeopid[670].grfeopid = 146;
	g_aeopid[670].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, oidEnemyExternal));
	g_aeopid[670].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, oidEnemyExternal));
	g_aeopid[670].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, oidEnemyExternal));

	g_aeopid[671].otyp = OTYP_So;
	g_aeopid[671].grfeopid = 36;
	g_aeopid[671].optdat.pvThunkFnUser = (PVTHUNKFN)GetStepguardEnemy;
	g_aeopid[671].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetStepguardEnemyObject);

	g_aeopid[672].grfeopid = 146;
	g_aeopid[672].otyp = OTYP_Bool;
	g_aeopid[672].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fRebindEnemy));
	g_aeopid[672].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fRebindEnemy));
	g_aeopid[672].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, fRebindEnemy));

	g_aeopid[673].grfeopid = 146;
	g_aeopid[673].otyp = OTYP_Bool;
	g_aeopid[673].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, fFlying));
	g_aeopid[673].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fFlying));
	g_aeopid[673].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fFlying));

	g_aeopid[674].otyp = OTYP_Float;
	g_aeopid[674].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, dzFlying));
	g_aeopid[674].grfeopid = 146;
	g_aeopid[674].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, dzFlying));
	g_aeopid[674].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, dzFlying));

	g_aeopid[675].otyp = OTYP_Clq;
	g_aeopid[675].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, clqSToUPursue));
	g_aeopid[675].grfeopid = 146;
	g_aeopid[675].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, clqSToUPursue));
	g_aeopid[675].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, clqSToUPursue));

	g_aeopid[676].otyp = OTYP_Lgns;
	g_aeopid[676].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, rpk));
	g_aeopid[676].grfeopid = 146;
	g_aeopid[676].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, rpk));
	g_aeopid[676].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, rpk));

	g_aeopid[677].otyp = OTYP_Void;
	g_aeopid[677].grfeopid = 1;
	g_aeopid[677].optdat.ibGet = 0;
	g_aeopid[677].optdat.ibSet = 1;
	g_aeopid[677].optdat.pvThunkFn = RefThunkStepguardSET_PATROL_ANIMATION;

	g_aeopid[678].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, ccoin));
	g_aeopid[678].otyp = OTYP_Int;
	g_aeopid[678].grfeopid = 146;
	g_aeopid[678].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, ccoin));
	g_aeopid[678].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, ccoin));

	g_aeopid[679].otyp = OTYP_Vector;
	g_aeopid[679].grfeopid = 146;
	g_aeopid[679].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, posBreakLocal));
	g_aeopid[679].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, posBreakLocal));
	g_aeopid[679].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, posBreakLocal));

	g_aeopid[680].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, fNoDifficultyUpdate));
	g_aeopid[680].otyp = OTYP_Bool;
	g_aeopid[680].grfeopid = 146;
	g_aeopid[680].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fNoDifficultyUpdate));
	g_aeopid[680].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fNoDifficultyUpdate));

	g_aeopid[681].grfeopid = 256;
	g_aeopid[681].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&UseStepguardExpl);
	g_aeopid[681].otyp = OTYP_Oid;

	g_aeopid[682].grfeopid = 256;
	g_aeopid[682].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&UseStepguardRwm);
	g_aeopid[682].otyp = OTYP_Oid;

	g_aeopid[683].grfeopid = 128;
	g_aeopid[683].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, uIdleStart));
	g_aeopid[683].otyp = OTYP_Float;

	g_aeopid[684].grfeopid = 146;
	g_aeopid[684].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, uFling));
	g_aeopid[684].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, uFling));
	g_aeopid[684].otyp = OTYP_Float;
	g_aeopid[684].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, uFling));

	g_aeopid[685].grfeopid = 146;
	g_aeopid[685].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fAutoFadeDying));
	g_aeopid[685].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, fAutoFadeDying));
	g_aeopid[685].otyp = OTYP_Bool;
	g_aeopid[685].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fAutoFadeDying));

	g_aeopid[686].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, sgs));
	g_aeopid[686].grfeopid = 2;
	g_aeopid[686].otyp = OTYP_Apek;

	g_aeopid[687].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, fBypassEffectHitSuppression));
	g_aeopid[687].grfeopid = 146;
	g_aeopid[687].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fBypassEffectHitSuppression));
	g_aeopid[687].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fBypassEffectHitSuppression));
	g_aeopid[687].otyp = OTYP_Bool;

	g_aeopid[688].otyp = OTYP_Bool;
	g_aeopid[688].grfeopid = 4;
	g_aeopid[688].optdat.pvThunkFnUser = (PVTHUNKFNUSER)GetStepguardOnFinalPathSegment;

	g_aeopid[689].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, lmDtTaunt));
	g_aeopid[689].otyp = OTYP_Lm;
	g_aeopid[689].grfeopid = 146;
	g_aeopid[689].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, lmDtTaunt));
	g_aeopid[689].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, lmDtTaunt));

	g_aeopid[690].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, fFaceDamageDirection));
	g_aeopid[690].grfeopid = 146;
	g_aeopid[690].otyp = OTYP_Bool;
	g_aeopid[690].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fFaceDamageDirection));
	g_aeopid[690].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fFaceDamageDirection));

	g_aeopid[691].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, muContact));
	g_aeopid[691].grfeopid = 146;
	g_aeopid[691].otyp = OTYP_Float;
	g_aeopid[691].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, muContact));
	g_aeopid[691].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, muContact));

	g_aeopid[692].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, fAbsorbDamage));
	g_aeopid[692].grfeopid = 146;
	g_aeopid[692].otyp = OTYP_Bool;
	g_aeopid[692].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fAbsorbDamage));
	g_aeopid[692].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fAbsorbDamage));

	g_aeopid[693].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, fAlignToGround));
	g_aeopid[693].grfeopid = 146;
	g_aeopid[693].otyp = OTYP_Bool;
	g_aeopid[693].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fAlignToGround));
	g_aeopid[693].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, fAlignToGround));

	g_aeopid[694].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddSggGuardName);
	g_aeopid[694].grfeopid = 256;
	g_aeopid[694].otyp = OTYP_Oid;

	g_aeopid[695].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddSggSearchXfmName);
	g_aeopid[695].grfeopid = 256;
	g_aeopid[695].otyp = OTYP_Oid;

	g_aeopid[696].otyp = OTYP_Float;
	g_aeopid[696].grfeopid = 146;
	g_aeopid[696].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, sDetectNear));
	g_aeopid[696].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SMARTGUARD, sDetectNear));
	g_aeopid[696].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, sDetectNear));

	g_aeopid[697].otyp = OTYP_Float;
	g_aeopid[697].grfeopid = 146;
	g_aeopid[697].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SMARTGUARD, zDetectNear));
	g_aeopid[697].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, zDetectNear));
	g_aeopid[697].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, zDetectNear));

	g_aeopid[698].otyp = OTYP_Float;
	g_aeopid[698].grfeopid = 146;
	g_aeopid[698].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SMARTGUARD, sSneak));
	g_aeopid[698].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, sSneak));
	g_aeopid[698].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, sSneak));

	g_aeopid[699].otyp = OTYP_Float;
	g_aeopid[699].grfeopid = 146;
	g_aeopid[699].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SMARTGUARD, zSneak));
	g_aeopid[699].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, zSneak));
	g_aeopid[699].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, zSneak));

	g_aeopid[700].otyp = OTYP_Oid;
	g_aeopid[700].grfeopid = 146;
	g_aeopid[700].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SMARTGUARD, oidSneakVol));
	g_aeopid[700].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, oidSneakVol));
	g_aeopid[700].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, oidSneakVol));

	g_aeopid[701].otyp = OTYP_Oid;
	g_aeopid[701].optdat.ibSetUser = static_cast<std::intptr_t>(
		offsetof(SMARTGUARD, sgf) + offsetof(SGF, oidFlash));
	g_aeopid[701].grfeopid = 128;

	g_aeopid[702].otyp = OTYP_Key;
	g_aeopid[702].optdat.ibGet = static_cast<std::intptr_t>(
		offsetof(SMARTGUARD, sgf) + offsetof(SGF, pflash));
	g_aeopid[702].grfeopid = 2;

	g_aeopid[703].otyp = OTYP_Void;
	g_aeopid[703].optdat.pvThunkFnUser = RefThunkSmartguardUSE_FLASHLIGHT_TARGET;
	g_aeopid[703].grfeopid = 1024;
	g_aeopid[703].optdat.pvThunkFn = nullptr;
	g_aeopid[703].optdat.ibSet = 2;

	g_aeopid[704].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SMARTGUARD, unk0));
	g_aeopid[704].otyp = OTYP_Bool;
	g_aeopid[704].grfeopid = 146;
	g_aeopid[704].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, unk0));
	g_aeopid[704].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, unk0));

	g_aeopid[705].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SMARTGUARD, fNoDetect));
	g_aeopid[705].otyp = OTYP_Bool;
	g_aeopid[705].grfeopid = 146;
	g_aeopid[705].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, fNoDetect));
	g_aeopid[705].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, fNoDetect));

	g_aeopid[706].otyp = 4197;
	g_aeopid[706].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SMARTGUARD, grfDetection));
	g_aeopid[706].grfeopid = 146;
	g_aeopid[706].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, grfDetection));
	g_aeopid[706].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SMARTGUARD, grfDetection));

	g_aeopid[707].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEPGUARD, ovrSgs) + offsetof(OVR, oid));
	g_aeopid[707].otyp = OTYP_Oid;
	g_aeopid[707].grfeopid = 146;
	g_aeopid[707].optdat.ibSet = static_cast<std::intptr_t>(offsetof(STEPGUARD, ovrSgs) + offsetof(OVR, oid));
	g_aeopid[707].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEPGUARD, ovrSgs) + offsetof(OVR, oid));

	g_aeopid[708].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddSmartguardBarrier);
	g_aeopid[708].otyp = OTYP_Oid;
	g_aeopid[708].grfeopid = 256;

	g_aeopid[708].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddSmartguardBarrier);
	g_aeopid[708].otyp = OTYP_Oid;
	g_aeopid[708].grfeopid = 256;

	g_aeopid[708].otyp = OTYP_Oid;
	g_aeopid[708].grfeopid = 0x100;
	g_aeopid[708].optdat.pfnset = (PFNRAW)AddSmartguardBarrier;
	g_aeopid[708].optdat.pfnsetUser = (PFNRAW)AddSmartguardBarrier;

	g_aeopid[709].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(STEP, dzBase));
	g_aeopid[709].otyp = OTYP_Float;
	g_aeopid[709].grfeopid = 2176;
	g_aeopid[709].optdat.fDef = 1108082688;

	g_aeopid[710].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetStepSpinRatio);
	g_aeopid[710].otyp = OTYP_Float;
	g_aeopid[710].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetStepSpinRatio);
	g_aeopid[710].grfeopid = 288;

	g_aeopid[711].otyp = OTYP_Float;
	g_aeopid[711].grfeopid = 290;
	g_aeopid[711].optdat.ibGet = static_cast<std::intptr_t>(offsetof(STEP, rGravity));
	g_aeopid[711].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetStepGravityRatio);
	g_aeopid[711].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetStepGravityRatio);

	g_aeopid[712].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BOMB, sDetonateRadius));
	g_aeopid[712].grfeopid = 146;
	g_aeopid[712].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BOMB, sDetonateRadius));
	g_aeopid[712].otyp = OTYP_Float;
	g_aeopid[712].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BOMB, sDetonateRadius));

	g_aeopid[713].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BOMB, sDetonateRadiusJt));
	g_aeopid[713].grfeopid = 146;
	g_aeopid[713].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BOMB, sDetonateRadiusJt));
	g_aeopid[713].otyp = OTYP_Float;
	g_aeopid[713].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BOMB, sDetonateRadiusJt));

	g_aeopid[714].grfeopid = 146;
	g_aeopid[714].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BOMB, bdt));
	g_aeopid[714].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BOMB, bdt));
	g_aeopid[714].otyp = OTYP_Alarms;
	g_aeopid[714].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BOMB, bdt));

	g_aeopid[715].grfeopid = 146;
	g_aeopid[715].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BOMB, bdm));
	g_aeopid[715].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BOMB, bdm));
	g_aeopid[715].otyp = OTYP_Altk;
	g_aeopid[715].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BOMB, bdm));

	g_aeopid[716].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BOMB, fTouchDamage));
	g_aeopid[716].grfeopid = 146;
	g_aeopid[716].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BOMB, fTouchDamage));
	g_aeopid[716].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BOMB, fTouchDamage));
	g_aeopid[716].otyp = OTYP_Bool;

	g_aeopid[717].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BOMB, sdvMax));
	g_aeopid[717].grfeopid = 146;
	g_aeopid[717].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BOMB, sdvMax));
	g_aeopid[717].otyp = OTYP_Float;
	g_aeopid[717].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BOMB, sdvMax));

	g_aeopid[718].otyp = OTYP_Float;
	g_aeopid[718].grfeopid = 146;
	g_aeopid[718].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BOMB, mMax));
	g_aeopid[718].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BOMB, mMax));
	g_aeopid[718].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BOMB, mMax));

	g_aeopid[719].otyp = OTYP_Lm;
	g_aeopid[719].grfeopid = 146;
	g_aeopid[719].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BOMB, lmMass));
	g_aeopid[719].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BOMB, lmMass));
	g_aeopid[719].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BOMB, lmMass));

	g_aeopid[720].otyp = OTYP_Float;
	g_aeopid[720].grfeopid = 146;
	g_aeopid[720].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BOMB, rExplodeScale));
	g_aeopid[720].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BOMB, rExplodeScale));
	g_aeopid[720].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BOMB, rExplodeScale));

	g_aeopid[721].otyp = OTYP_Void;
	g_aeopid[721].grfeopid = 1;
	g_aeopid[721].optdat.ibSet = 1;
	g_aeopid[721].optdat.ibGet = 0;
	g_aeopid[721].optdat.pvThunkFn = RefThunkBombPRIME;

	g_aeopid[722].otyp = OTYP_Void;
	g_aeopid[722].grfeopid = 1;
	g_aeopid[722].optdat.ibGet = 0;
	g_aeopid[722].optdat.ibSet = 0;
	g_aeopid[722].optdat.pvThunkFn = RefThunkBombDETONATE;

	g_aeopid[723].grfeopid = 146;
	g_aeopid[723].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BOMB, fExplodeEffects));
	g_aeopid[723].otyp = OTYP_Bool;
	g_aeopid[723].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BOMB, fExplodeEffects));
	g_aeopid[723].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BOMB, fExplodeEffects));

	g_aeopid[724].grfeopid = 146;
	g_aeopid[724].otyp = OTYP_Bool;
	g_aeopid[724].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BOMB, fReclaim));
	g_aeopid[724].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BOMB, fReclaim));
	g_aeopid[724].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BOMB, fReclaim));

	g_aeopid[725].otyp = OTYP_Fapl;
	g_aeopid[725].grfeopid = 146;
	g_aeopid[725].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BOMB, zpk));
	g_aeopid[725].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BOMB, zpk));
	g_aeopid[725].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BOMB, zpk));

	g_aeopid[726].otyp = OTYP_Sfxid;
	g_aeopid[726].grfeopid = 4242;
	g_aeopid[726].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SFX, sfxid));
	g_aeopid[726].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SFX, sfxid));
	g_aeopid[726].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SFX, sfxid));
	g_aeopid[726].optdat.pfnensure = (PFNENSURE)PsfxEnsureBomb;

	g_aeopid[727].otyp = OTYP_Float;
	g_aeopid[727].grfeopid = 4242;
	g_aeopid[727].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SFX, sStart));
	g_aeopid[727].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SFX, sStart));
	g_aeopid[727].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SFX, sStart));
	g_aeopid[727].optdat.pfnensure = (PFNENSURE)PsfxEnsureBomb;

	g_aeopid[728].otyp = OTYP_Float;
	g_aeopid[728].grfeopid = 4242;
	g_aeopid[728].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SFX, sFull));
	g_aeopid[728].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SFX, sFull));
	g_aeopid[728].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SFX, sFull));
	g_aeopid[728].optdat.pfnensure = (PFNENSURE)PsfxEnsureBomb;

	g_aeopid[729].otyp = OTYP_Float;
	g_aeopid[729].grfeopid = 4242;
	g_aeopid[729].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SFX, uVol));
	g_aeopid[729].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SFX, uVol));
	g_aeopid[729].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SFX, uVol));
	g_aeopid[729].optdat.pfnensure = (PFNENSURE)PsfxEnsureBomb;

	g_aeopid[730].otyp = OTYP_Float;
	g_aeopid[730].grfeopid = 146;
	g_aeopid[730].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BOMB, svDetonate));
	g_aeopid[730].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BOMB, svDetonate));
	g_aeopid[730].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BOMB, svDetonate));

	g_aeopid[731].grfeopid = 130;
	g_aeopid[731].otyp = OTYP_Bool;
	g_aeopid[731].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(BOMB, fCustomXps));
	g_aeopid[731].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BOMB, fCustomXps));

	g_aeopid[732].grfeopid = 1;
	g_aeopid[732].otyp = OTYP_Void;
	g_aeopid[732].optdat.ibGet = 0;
	g_aeopid[732].optdat.ibSet = 3;
	g_aeopid[732].optdat.pvThunkFn = RefThunkJtTHROW;

	g_aeopid[733].grfeopid = 1;
	g_aeopid[733].otyp = OTYP_Void;
	g_aeopid[733].optdat.ibGet = 0;
	g_aeopid[733].optdat.pvThunkFn = RefThunkJtUNHOOK;
	g_aeopid[733].optdat.ibSet = 0;

	g_aeopid[734].grfeopid = 1;
	g_aeopid[734].otyp = OTYP_Void;
	g_aeopid[734].optdat.ibGet = 0;
	g_aeopid[734].optdat.ibSet = 1;
	//g_aeopid[734].optdat.pvThunkFn = RefThunkJtPROFILE;

	g_aeopid[735].grfeopid = 146;
	g_aeopid[735].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JT, fRenderCaneTrail));
	g_aeopid[735].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JT, fRenderCaneTrail));
	g_aeopid[735].otyp = OTYP_Bool;
	g_aeopid[735].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JT, fRenderCaneTrail));

	g_aeopid[736].grfeopid = 34;
	g_aeopid[736].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JT, jtpdk));
	g_aeopid[736].otyp = OTYP_Lootek;
	g_aeopid[736].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetJtJtpdk);

	g_aeopid[737].optdat.pvThunkFn = RefThunkJtPLACE_ON_PIPE;
	g_aeopid[737].grfeopid = 1;
	g_aeopid[737].otyp = OTYP_Void;
	g_aeopid[737].optdat.ibGet = 0;
	g_aeopid[737].optdat.ibSet = 1;

	g_aeopid[738].otyp = OTYP_Void;
	g_aeopid[738].grfeopid = 1;
	g_aeopid[738].optdat.ibGet = 0;
	g_aeopid[738].optdat.pvThunkFn = RefThunkJtSET_PUPPET;
	g_aeopid[738].optdat.crefReq = 1;

	g_aeopid[739].otyp = OTYP_Void;
	g_aeopid[739].grfeopid = 1;
	g_aeopid[739].optdat.ibSet = 1;
	g_aeopid[739].optdat.pvThunkFn = RefThunkJtSET_STUN;
	g_aeopid[739].optdat.ibGet = 0;

	g_aeopid[740].otyp = OTYP_Void;
	g_aeopid[740].grfeopid = 1;
	g_aeopid[740].optdat.pvThunkFn = RefThunkJtDEPLOY_MINE;
	g_aeopid[740].optdat.ibGet = 0;
	g_aeopid[740].optdat.ibSet = 0;

	g_aeopid[741].grfeopid = 146;
	g_aeopid[741].otyp = OTYP_Bool;
	g_aeopid[741].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JT, fCancelPeek));
	g_aeopid[741].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JT, fCancelPeek));
	g_aeopid[741].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JT, fCancelPeek));

	g_aeopid[742].grfeopid = 162;
	g_aeopid[742].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetJtDisableStepCustomXps);
	g_aeopid[742].otyp = OTYP_Bool;
	g_aeopid[742].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JT, fDisableStepCustomXps));
	g_aeopid[742].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JT, fDisableStepCustomXps));

	g_aeopid[743].optdat.pvThunkFn = RefThunkJtTRIGGER_LOCKG;
	g_aeopid[743].otyp = OTYP_Void;
	g_aeopid[743].grfeopid = 1;
	g_aeopid[743].optdat.ibSet = 1;
	g_aeopid[743].optdat.ibGet = 0;

	g_aeopid[744].optdat.pvThunkFn = RefThunkJtTRIGGER_DISPLACE;
	g_aeopid[744].otyp = OTYP_Void;
	g_aeopid[744].grfeopid = 1;
	g_aeopid[744].optdat.ibGet = 0;
	g_aeopid[744].optdat.ibSet = 1;

	g_aeopid[745].otyp = OTYP_Void;
	g_aeopid[745].optdat.ibSet = 1;
	g_aeopid[745].optdat.pvThunkFn = RefThunkJtTRIGGER_DISPLACE_SECONDARY;
	g_aeopid[745].grfeopid = 1;
	g_aeopid[745].optdat.ibGet = 0;

	g_aeopid[746].grfeopid = 128;
	g_aeopid[746].otyp = OTYP_Brkps;
	g_aeopid[746].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RCHM, rchmk));

	g_aeopid[747].grfeopid = 128;
	g_aeopid[747].otyp = OTYP_Oid;
	g_aeopid[747].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RCHM, oidHost));

	g_aeopid[748].grfeopid = 128;
	g_aeopid[748].otyp = OTYP_Oid;
	g_aeopid[748].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RCHM, oidTouch));

	g_aeopid[749].grfeopid = 128;
	g_aeopid[749].otyp = OTYP_Icek;
	g_aeopid[749].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RCHM, grftak));

	g_aeopid[750].grfeopid = 1024;
	g_aeopid[750].otyp = OTYP_Void;
	g_aeopid[750].optdat.pvThunkFnUser = RefThunkRchmSET_NATURAL_COEFFICIENTS;
	g_aeopid[750].optdat.ibSet = 3;
	g_aeopid[750].optdat.pvThunkFn = nullptr;

	g_aeopid[751].grfeopid = 1024;
	g_aeopid[751].otyp = OTYP_Void;
	g_aeopid[751].optdat.pvThunkFnUser = RefThunkRchmSET_CENTER_COEFFICIENTS;
	g_aeopid[751].optdat.ibSet = 3;
	g_aeopid[751].optdat.pvThunkFn = nullptr;

	g_aeopid[752].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAT, sScurry));
	g_aeopid[752].grfeopid = 2194;
	g_aeopid[752].optdat.fDef = 1140457472;
	g_aeopid[752].otyp = OTYP_Float;
	g_aeopid[752].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAT, sScurry));
	g_aeopid[752].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAT, sScurry));

	g_aeopid[753].otyp = OTYP_Float;
	g_aeopid[753].grfeopid = 2194;
	g_aeopid[753].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAT, svScurry));
	g_aeopid[753].optdat.fDef = 1140457472;
	g_aeopid[753].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAT, svScurry));
	g_aeopid[753].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAT, svScurry));

	g_aeopid[754].otyp = OTYP_Float;
	g_aeopid[754].grfeopid = 2194;
	g_aeopid[754].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAT, dsFrameScurry));
	g_aeopid[754].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAT, dsFrameScurry));
	g_aeopid[754].optdat.fDef = 1097859072;
	g_aeopid[754].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAT, dsFrameScurry));

	g_aeopid[755].otyp = OTYP_Float;
	g_aeopid[755].grfeopid = 2194;
	g_aeopid[755].optdat.fDef = 1148846080;
	g_aeopid[755].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAT, svFlee));
	g_aeopid[755].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAT, svFlee));
	g_aeopid[755].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAT, svFlee));

	g_aeopid[756].otyp = OTYP_Float;
	g_aeopid[756].grfeopid = 2194;
	g_aeopid[756].optdat.fDef = 1097859072;
	g_aeopid[756].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAT, dsFrameScurry));
	g_aeopid[756].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAT, dsFrameScurry));
	g_aeopid[756].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAT, dsFrameScurry));

	g_aeopid[757].otyp = OTYP_Float;
	g_aeopid[757].grfeopid = 2194;
	g_aeopid[757].optdat.fDef = 1161527296;
	g_aeopid[757].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAT, sGroupRadius));
	g_aeopid[757].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAT, sGroupRadius));
	g_aeopid[757].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAT, sGroupRadius));

	g_aeopid[758].otyp = OTYP_Float;
	g_aeopid[758].grfeopid = 146;
	g_aeopid[758].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAT, lmDtStop));
	g_aeopid[758].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAT, lmDtStop));
	g_aeopid[758].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAT, lmDtStop));

	g_aeopid[759].otyp = OTYP_Float;
	g_aeopid[759].grfeopid = 2194;
	g_aeopid[759].optdat.fDef = 1133903872;
	g_aeopid[759].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAT, svLeap));
	g_aeopid[759].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAT, svLeap));
	g_aeopid[759].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAT, svLeap));

	g_aeopid[760].otyp = OTYP_Flys;
	g_aeopid[760].grfeopid = 2;
	g_aeopid[760].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAT, rats));

	g_aeopid[761].otyp = OTYP_Int;
	g_aeopid[761].grfeopid = 146;
	g_aeopid[761].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAT, ccoin));
	g_aeopid[761].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAT, ccoin));
	g_aeopid[761].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAT, ccoin));

	g_aeopid[762].grfeopid = 146;
	g_aeopid[762].otyp = OTYP_Bool;
	g_aeopid[762].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAT, fSuppressDifficultyUpdate));
	g_aeopid[762].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAT, fSuppressDifficultyUpdate));
	g_aeopid[762].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAT, fSuppressDifficultyUpdate));

	g_aeopid[763].otyp = OTYP_Float;
	g_aeopid[763].grfeopid = 2176;
	g_aeopid[763].optdat.fDef = 1112014848;
	g_aeopid[763].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TAIL, rSpring));

	g_aeopid[764].otyp = OTYP_Float;
	g_aeopid[764].grfeopid = 2176;
	g_aeopid[764].optdat.fDef = 1103626240;
	g_aeopid[764].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TAIL, rDampingWorld));

	g_aeopid[765].otyp = OTYP_Float;
	g_aeopid[765].grfeopid = 2176;
	g_aeopid[765].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TAIL, rDampingLocal));
	g_aeopid[765].optdat.fDef = 0;

	g_aeopid[766].otyp = OTYP_Float;
	g_aeopid[766].grfeopid = 2176;
	g_aeopid[766].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TAIL, radMax));
	g_aeopid[766].optdat.fDef = 1080033280;

	g_aeopid[767].otyp = OTYP_Int;
	g_aeopid[767].grfeopid = 2176;
	g_aeopid[767].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TAIL, cIterate));
	g_aeopid[767].optdat.fDef = 1;

	g_aeopid[768].otyp = OTYP_Vector;
	g_aeopid[768].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TAIL, dvGravity));
	g_aeopid[768].grfeopid = 146;
	g_aeopid[768].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TAIL, dvGravity));
	g_aeopid[768].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TAIL, dvGravity));

	g_aeopid[769].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TAIL, fUnlockRot));
	g_aeopid[769].otyp = OTYP_Bool;
	g_aeopid[769].grfeopid = 146;
	g_aeopid[769].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TAIL, fUnlockRot));
	g_aeopid[769].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TAIL, fUnlockRot));

	g_aeopid[770].otyp = OTYP_Vector;
	g_aeopid[770].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TAIL, posTip));
	g_aeopid[770].grfeopid = 146;
	g_aeopid[770].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TAIL, posTip));
	g_aeopid[770].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TAIL, posTip));

	g_aeopid[770].optdat.ibGet = 800;
	g_aeopid[770].optdat.ibSet = 800;

	g_aeopid[771].otyp = OTYP_Oid;
	g_aeopid[771].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetRobCollectible);
	g_aeopid[771].grfeopid = 256;

	g_aeopid[772].otyp = OTYP_Oid;
	g_aeopid[772].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetRobHarvester);
	g_aeopid[772].grfeopid = 256;

	g_aeopid[773].otyp = OTYP_Oid;
	g_aeopid[773].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetRobVehicle);
	g_aeopid[773].grfeopid = 256;

	g_aeopid[774].otyp = OTYP_Oid;
	g_aeopid[774].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetRobPrize);
	g_aeopid[774].grfeopid = 256;

	g_aeopid[775].otyp = OTYP_Oid;
	g_aeopid[775].grfeopid = 256;
	g_aeopid[775].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddRobSpawnTunnel);

	g_aeopid[776].otyp = OTYP_Float;
	g_aeopid[776].grfeopid = 4242;
	g_aeopid[776].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, crocActiveMaxEasy));
	g_aeopid[776].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[776].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, crocActiveMaxEasy));
	g_aeopid[776].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, crocActiveMaxEasy));

	g_aeopid[777].otyp = OTYP_Float;
	g_aeopid[777].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, crocActiveMaxHard));
	g_aeopid[777].grfeopid = 4226;
	g_aeopid[777].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, crocActiveMaxHard));
	g_aeopid[777].optdat.pfnensure = (PFNENSURE)ProddCurRob;

	g_aeopid[778].otyp = OTYP_Clq;
	g_aeopid[778].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, clqCrocActive));
	g_aeopid[778].grfeopid = 4242;
	g_aeopid[778].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, clqCrocActive));
	g_aeopid[778].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[778].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, clqCrocActive));

	g_aeopid[779].otyp = OTYP_Float;
	g_aeopid[779].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, crohActiveMaxEasy));
	g_aeopid[779].grfeopid = 4242;
	g_aeopid[779].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, crohActiveMaxEasy));
	g_aeopid[779].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[779].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, crohActiveMaxEasy));

	g_aeopid[780].otyp = OTYP_Float;
	g_aeopid[780].grfeopid = 4226;
	g_aeopid[780].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, crohActiveMaxHard));
	g_aeopid[780].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, crohActiveMaxHard));
	g_aeopid[780].optdat.pfnensure = (PFNENSURE)ProddCurRob;

	g_aeopid[781].otyp = OTYP_Clq;
	g_aeopid[781].grfeopid = 4242;
	g_aeopid[781].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, clqCrohActive));
	g_aeopid[781].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, clqCrohActive));
	g_aeopid[781].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, clqCrohActive));
	g_aeopid[781].optdat.pfnensure = (PFNENSURE)ProddCurRob;

	g_aeopid[782].otyp = OTYP_Lm;
	g_aeopid[782].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, lmDtAddRocEasy));
	g_aeopid[782].grfeopid = 4242;
	g_aeopid[782].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, lmDtAddRocEasy));
	g_aeopid[782].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[782].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, lmDtAddRocEasy));

	g_aeopid[783].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, lmDtAddRocHard));
	g_aeopid[783].otyp = OTYP_Lm;
	g_aeopid[783].grfeopid = 4242;
	g_aeopid[783].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, lmDtAddRocHard));
	g_aeopid[783].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[783].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, lmDtAddRocHard));

	g_aeopid[784].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, clqDtAddRoc));
	g_aeopid[784].otyp = OTYP_Clq;
	g_aeopid[784].grfeopid = 4242;
	g_aeopid[784].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, clqDtAddRoc));
	g_aeopid[784].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[784].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, clqDtAddRoc));

	g_aeopid[785].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, lmDtAddRohEasy));
	g_aeopid[785].otyp = OTYP_Lm;
	g_aeopid[785].grfeopid = 4242;
	g_aeopid[785].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, lmDtAddRohEasy));
	g_aeopid[785].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[785].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, lmDtAddRohEasy));

	g_aeopid[786].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, lmDtAddRohHard));
	g_aeopid[786].otyp = OTYP_Lm;
	g_aeopid[786].grfeopid = 4242;
	g_aeopid[786].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, lmDtAddRohHard));
	g_aeopid[786].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[786].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, lmDtAddRohHard));

	g_aeopid[787].otyp = OTYP_Clq;
	g_aeopid[787].grfeopid = 4242;
	g_aeopid[787].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, clqDtAddRoh));
	g_aeopid[787].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, clqDtAddRoh));
	g_aeopid[787].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[787].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, clqDtAddRoh));

	g_aeopid[788].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, svRohMaxEasy));
	g_aeopid[788].grfeopid = 4242;
	g_aeopid[788].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, svRohMaxEasy));
	g_aeopid[788].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[788].otyp = OTYP_Float;
	g_aeopid[788].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, svRohMaxEasy));

	g_aeopid[789].grfeopid = 4242;
	g_aeopid[789].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, svRohMaxHard));
	g_aeopid[789].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[789].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, svRohMaxHard));
	g_aeopid[789].otyp = OTYP_Float;
	g_aeopid[789].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, svRohMaxHard));

	g_aeopid[790].grfeopid = 4242;
	g_aeopid[790].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, clqSvRoh));
	g_aeopid[790].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[790].otyp = OTYP_Clq;
	g_aeopid[790].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, clqSvRoh));
	g_aeopid[790].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, clqSvRoh));

	g_aeopid[791].grfeopid = 4242;
	g_aeopid[791].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, dradRohMaxEasy));
	g_aeopid[791].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[791].otyp = OTYP_Float;
	g_aeopid[791].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, dradRohMaxEasy));
	g_aeopid[791].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, dradRohMaxEasy));

	g_aeopid[792].grfeopid = 4242;
	g_aeopid[792].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, dradRohMaxHard));
	g_aeopid[792].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[792].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, dradRohMaxHard));
	g_aeopid[792].otyp = OTYP_Float;
	g_aeopid[792].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, dradRohMaxHard));

	g_aeopid[793].otyp = OTYP_Clq;
	g_aeopid[793].grfeopid = 4242;
	g_aeopid[793].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RODD, clqDradRoh));
	g_aeopid[793].optdat.pfnensure = (PFNENSURE)ProddCurRob;
	g_aeopid[793].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RODD, clqDradRoh));
	g_aeopid[793].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RODD, clqDradRoh));

	g_aeopid[794].otyp = OTYP_Vector;
	g_aeopid[794].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROB, posCenterArena));
	g_aeopid[794].grfeopid = 146;
	g_aeopid[794].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROB, posCenterArena));
	g_aeopid[794].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ROB, posCenterArena));

	g_aeopid[795].otyp = OTYP_Float;
	g_aeopid[795].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROB, sRadiusArenaTube));
	g_aeopid[795].grfeopid = 146;
	g_aeopid[795].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROB, sRadiusArenaTube));
	g_aeopid[795].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ROB, sRadiusArenaTube));

	g_aeopid[796].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROB, sRadiusArenaRoc));
	g_aeopid[796].otyp = OTYP_Float;
	g_aeopid[796].grfeopid = 146;
	g_aeopid[796].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROB, sRadiusArenaRoc));
	g_aeopid[796].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ROB, sRadiusArenaRoc));

	g_aeopid[797].otyp = OTYP_Float;
	g_aeopid[797].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROB, sRadiusArenaWander));
	g_aeopid[797].grfeopid = 146;
	g_aeopid[797].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROB, sRadiusArenaWander));
	g_aeopid[797].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ROB, sRadiusArenaWander));

	g_aeopid[798].otyp = OTYP_Float;
	g_aeopid[798].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROB, rAspectArena));
	g_aeopid[798].grfeopid = 146;
	g_aeopid[798].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROB, rAspectArena));
	g_aeopid[798].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ROB, rAspectArena));

	g_aeopid[799].otyp = OTYP_Float;
	g_aeopid[799].grfeopid = 146;
	g_aeopid[799].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROB, sCollectibleDropHeight));
	g_aeopid[799].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ROB, sCollectibleDropHeight));
	g_aeopid[799].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROB, sCollectibleDropHeight));

	g_aeopid[800].otyp = OTYP_Int;
	g_aeopid[800].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROB, crocCreateMax));
	g_aeopid[800].grfeopid = 130;
	g_aeopid[800].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROB, crocCreateMax));

	g_aeopid[801].otyp = OTYP_Blipmk;
	g_aeopid[801].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROB, robk));
	g_aeopid[801].grfeopid = 130;
	g_aeopid[801].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROB, robk));

	g_aeopid[802].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROB, irodd));
	g_aeopid[802].otyp = OTYP_Int;
	g_aeopid[802].grfeopid = 146;
	g_aeopid[802].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROB, irodd));
	g_aeopid[802].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ROB, irodd));

	g_aeopid[803].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROB, clqTune));
	g_aeopid[803].otyp = OTYP_Clq;
	g_aeopid[803].grfeopid = 146;
	g_aeopid[803].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROB, clqTune));
	g_aeopid[803].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ROB, clqTune));

	g_aeopid[804].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROV, svMax));
	g_aeopid[804].otyp = OTYP_Float;
	g_aeopid[804].grfeopid = 146;
	g_aeopid[804].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROV, svMax));
	g_aeopid[804].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ROV, svMax));

	g_aeopid[805].otyp = OTYP_Lm;
	g_aeopid[805].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROH, lmDtWander));
	g_aeopid[805].grfeopid = 146;
	g_aeopid[805].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROH, lmDtWander));
	g_aeopid[805].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ROH, lmDtWander));

	g_aeopid[806].otyp = OTYP_Int;
	g_aeopid[806].grfeopid = 128;
	g_aeopid[806].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROC, cHitMax));

	g_aeopid[807].otyp = OTYP_Float;
	g_aeopid[807].grfeopid = 146;
	g_aeopid[807].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROC, dtRocsHitUp));
	g_aeopid[807].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ROC, dtRocsHitUp));
	g_aeopid[807].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROC, dtRocsHitUp));

	g_aeopid[808].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROC, dtRocsHitDown));
	g_aeopid[808].otyp = OTYP_Float;
	g_aeopid[808].grfeopid = 146;
	g_aeopid[808].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROC, dtRocsHitDown));
	g_aeopid[808].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ROC, dtRocsHitDown));

	g_aeopid[809].otyp = OTYP_Clq;
	g_aeopid[809].grfeopid = 146;
	g_aeopid[809].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ROC, clqBubble));
	g_aeopid[809].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ROC, clqBubble));
	g_aeopid[809].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ROC, clqBubble));

	g_aeopid[810].otyp = OTYP_Robk;
	g_aeopid[810].grfeopid = 34;
	g_aeopid[810].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DART, darts));
	g_aeopid[810].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetDartDarts);

	g_aeopid[811].optdat.fDef = 1159479296;
	g_aeopid[811].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DART, svDart));
	g_aeopid[811].otyp = OTYP_Float;
	g_aeopid[811].grfeopid = 2194;
	g_aeopid[811].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DART, svDart));
	g_aeopid[811].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DART, svDart));

	g_aeopid[812].optdat.fDef = 1101004800;
	g_aeopid[812].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DART, sRadiusFireCloud));
	g_aeopid[812].otyp = OTYP_Float;
	g_aeopid[812].grfeopid = 2194;
	g_aeopid[812].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DART, sRadiusFireCloud));
	g_aeopid[812].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DART, sRadiusFireCloud));

	g_aeopid[813].otyp = OTYP_Float;
	g_aeopid[813].grfeopid = 2194;
	g_aeopid[813].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DART, sRadiusStrikeCloud));
	g_aeopid[813].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DART, sRadiusStrikeCloud));
	g_aeopid[813].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DART, sRadiusStrikeCloud));
	g_aeopid[813].optdat.fDef = 1106247680;

	g_aeopid[814].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DART, dtFade));
	g_aeopid[814].optdat.fDef = 1069547520;
	g_aeopid[814].otyp = OTYP_Float;
	g_aeopid[814].grfeopid = 2194;
	g_aeopid[814].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DART, dtFade));
	g_aeopid[814].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DART, dtFade));

	g_aeopid[815].otyp = OTYP_Float;
	g_aeopid[815].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DART, dtMaxStuck));
	g_aeopid[815].grfeopid = 2194;
	g_aeopid[815].optdat.fDef = 1092616192;
	g_aeopid[815].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DART, dtMaxStuck));
	g_aeopid[815].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DART, dtMaxStuck));

	g_aeopid[816].otyp = OTYP_Oid;
	g_aeopid[816].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DARTGUN, oidDart));
	g_aeopid[816].grfeopid = 128;

	g_aeopid[817].otyp = OTYP_Int;
	g_aeopid[817].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DARTGUN, cDarts));
	g_aeopid[817].optdat.fDef = 5;
	g_aeopid[817].grfeopid = 2176;

	g_aeopid[818].otyp = OTYP_Float;
	g_aeopid[818].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DARTGUN, dtFireInterval));
	g_aeopid[818].grfeopid = 2194;
	g_aeopid[818].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DARTGUN, dtFireInterval));
	g_aeopid[818].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DARTGUN, dtFireInterval));
	g_aeopid[818].optdat.fDef = 1065353216;

	g_aeopid[819].otyp = OTYP_Float;
	g_aeopid[819].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DARTGUN, dzTarget));
	g_aeopid[819].optdat.fDef = 1101004800;
	g_aeopid[819].grfeopid = 2194;
	g_aeopid[819].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DARTGUN, dzTarget));
	g_aeopid[819].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DARTGUN, dzTarget));

	g_aeopid[820].otyp = OTYP_Oid;
	g_aeopid[820].grfeopid = 256;
	g_aeopid[820].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddDartgunRatTarget);

	g_aeopid[821].otyp = OTYP_Void;
	g_aeopid[821].optdat.pvThunkFn = RefThunkDartgunSTART_TARGET_AREA_CHANGE;
	g_aeopid[821].grfeopid = 1;
	g_aeopid[821].optdat.ibGet = 0;
	g_aeopid[821].optdat.ibSet = 0;

	g_aeopid[822].grfeopid = 1;
	g_aeopid[822].otyp = OTYP_Void;
	g_aeopid[822].optdat.ibGet = 0;
	g_aeopid[822].optdat.ibSet = 1;
	g_aeopid[822].optdat.pvThunkFn = RefThunkDartgunADD_TARGET_AREA_TARGET;

	g_aeopid[823].grfeopid = 2194;
	g_aeopid[823].optdat.ibGet = static_cast<std::intptr_t>(offsetof(MISSILE, svMissileLaunch));
	g_aeopid[823].otyp = OTYP_Float;
	g_aeopid[823].optdat.ibSet = static_cast<std::intptr_t>(offsetof(MISSILE, svMissileLaunch));
	g_aeopid[823].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(MISSILE, svMissileLaunch));
	g_aeopid[823].optdat.fDef = 1145569280;

	g_aeopid[824].grfeopid = 18;
	g_aeopid[824].optdat.ibGet = static_cast<std::intptr_t>(offsetof(MISSILE, paloTarget));
	g_aeopid[824].optdat.ibSet = static_cast<std::intptr_t>(offsetof(MISSILE, paloTarget));
	g_aeopid[824].otyp = OTYP_Alo;

	g_aeopid[825].grfeopid = 18;
	g_aeopid[825].optdat.ibGet = static_cast<std::intptr_t>(offsetof(MISSILE, posTargetLocal));
	g_aeopid[825].optdat.ibSet = static_cast<std::intptr_t>(offsetof(MISSILE, posTargetLocal));
	g_aeopid[825].otyp = OTYP_Vector;

	g_aeopid[826].grfeopid = 146;
	g_aeopid[826].optdat.ibGet = static_cast<std::intptr_t>(offsetof(MISSILE, fFollowTrajectory));
	g_aeopid[826].optdat.ibSet = static_cast<std::intptr_t>(offsetof(MISSILE, fFollowTrajectory));
	g_aeopid[826].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(MISSILE, fFollowTrajectory));
	g_aeopid[826].otyp = OTYP_Bool;

	g_aeopid[827].optdat.pvThunkFnUser = RefThunkMissileADD_IGNORE_OBJECT;
	g_aeopid[827].grfeopid = 1024;
	g_aeopid[827].otyp = OTYP_Void;
	g_aeopid[827].optdat.pvThunkFn = nullptr;
	g_aeopid[827].optdat.ibSet = 1;

	g_aeopid[828].grfeopid = 1024;
	g_aeopid[828].optdat.pvThunkFnUser = RefThunkMissileADD_IGNORE_CLASS;
	g_aeopid[828].otyp = OTYP_Void;
	g_aeopid[828].optdat.ibSet = 1;
	g_aeopid[828].optdat.pvThunkFn = nullptr;

	g_aeopid[829].grfeopid = 2194;
	g_aeopid[829].otyp = OTYP_Float;
	g_aeopid[829].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ACCMISS, svMissileMax));
	g_aeopid[829].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ACCMISS, svMissileMax));
	g_aeopid[829].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ACCMISS, svMissileMax));
	g_aeopid[829].optdat.fDef = 1157234688;

	g_aeopid[830].grfeopid = 2194;
	g_aeopid[830].otyp = OTYP_Float;
	g_aeopid[830].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ACCMISS, sdvMissileMax));
	g_aeopid[830].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ACCMISS, sdvMissileMax));
	g_aeopid[830].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(ACCMISS, sdvMissileMax));
	g_aeopid[830].optdat.fDef = 1167867904;

	g_aeopid[831].grfeopid = 18;
	g_aeopid[831].otyp = OTYP_Vector;
	g_aeopid[831].optdat.ibGet = static_cast<std::intptr_t>(offsetof(ACCMISS, vTarget));
	g_aeopid[831].optdat.ibSet = static_cast<std::intptr_t>(offsetof(ACCMISS, vTarget));

	g_aeopid[832].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TARMISS, dtLifetimeOverride));
	g_aeopid[832].grfeopid = 146;
	g_aeopid[832].otyp = OTYP_Float;
	g_aeopid[832].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TARMISS, dtLifetimeOverride));
	g_aeopid[832].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TARMISS, dtLifetimeOverride));

	g_aeopid[833].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TARMISS, svMax));
	g_aeopid[833].optdat.fDef = 1153138688;
	g_aeopid[833].grfeopid = 2194;
	g_aeopid[833].otyp = OTYP_Float;
	g_aeopid[833].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TARMISS, svMax));
	g_aeopid[833].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TARMISS, svMax));

	g_aeopid[834].otyp = OTYP_Float;
	g_aeopid[834].grfeopid = 2194;
	g_aeopid[834].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TARMISS, rTrajectoryBank));
	g_aeopid[834].optdat.fDef = 1065353216;
	g_aeopid[834].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TARMISS, rTrajectoryBank));
	g_aeopid[834].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TARMISS, rTrajectoryBank));

	g_aeopid[835].otyp = OTYP_Vector;
	g_aeopid[835].grfeopid = 146;
	g_aeopid[835].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TARMISS, vTarget));
	g_aeopid[835].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TARMISS, vTarget));
	g_aeopid[835].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TARMISS, vTarget));

	g_aeopid[836].grfeopid = 2194;
	g_aeopid[836].otyp = OTYP_Float;
	g_aeopid[836].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JMT, sRadius));
	g_aeopid[836].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JMT, sRadius));
	g_aeopid[836].optdat.fDef = 1120403456;
	g_aeopid[836].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JMT, sRadius));

	g_aeopid[837].grfeopid = 2176;
	g_aeopid[837].otyp = OTYP_Oid;
	g_aeopid[837].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JMT, oidSetIgnoreJt));
	g_aeopid[837].optdat.fDef = -1;

	g_aeopid[838].otyp = OTYP_Float;
	g_aeopid[838].grfeopid = 146;
	g_aeopid[838].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WAYPOINT, dtPause));
	g_aeopid[838].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WAYPOINT, dtPause));
	g_aeopid[838].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WAYPOINT, dtPause));

	g_aeopid[839].otyp = OTYP_Oid;
	g_aeopid[839].grfeopid = 130;
	g_aeopid[839].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WAYPOINT, oidSync));
	g_aeopid[839].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WAYPOINT, oidSync));

	g_aeopid[840].otyp = OTYP_Void;
	g_aeopid[840].grfeopid = 0x400;
	g_aeopid[840].optdat.ibSet = 4;
	g_aeopid[840].optdat.pvThunkFnUser = RefThunkWaypointSET_SM_GOAL;
	g_aeopid[840].optdat.pvThunkFn = nullptr;

	g_aeopid[841].otyp = OTYP_Float;
	g_aeopid[841].grfeopid = 146;
	g_aeopid[841].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(GOMER, sAbandon));
	g_aeopid[841].optdat.ibGet = static_cast<std::intptr_t>(offsetof(GOMER, sAbandon));
	g_aeopid[841].optdat.ibSet = static_cast<std::intptr_t>(offsetof(GOMER, sAbandon));

	g_aeopid[842].otyp = OTYP_Float;
	g_aeopid[842].grfeopid = 146;
	g_aeopid[842].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(GOMER, dzAbandon));
	g_aeopid[842].optdat.ibGet = static_cast<std::intptr_t>(offsetof(GOMER, dzAbandon));
	g_aeopid[842].optdat.ibSet = static_cast<std::intptr_t>(offsetof(GOMER, dzAbandon));

	g_aeopid[843].otyp = OTYP_Oid;
	g_aeopid[843].grfeopid = 128;
	g_aeopid[843].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(GOMER, oidAbandon));

	g_aeopid[844].otyp = OTYP_Rail;
	g_aeopid[844].grfeopid = 18;
	g_aeopid[844].optdat.ibGet = static_cast<std::intptr_t>(offsetof(GOMER, pvolAbandon));
	g_aeopid[844].optdat.ibSet = static_cast<std::intptr_t>(offsetof(GOMER, pvolAbandon));

	g_aeopid[845].otyp = OTYP_Lm;
	g_aeopid[845].grfeopid = 146;
	g_aeopid[845].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(GOMER, lmSDetect));
	g_aeopid[845].optdat.ibGet = static_cast<std::intptr_t>(offsetof(GOMER, lmSDetect));
	g_aeopid[845].optdat.ibSet = static_cast<std::intptr_t>(offsetof(GOMER, lmSDetect));

	g_aeopid[846].otyp = OTYP_Lm;
	g_aeopid[846].grfeopid = 146;
	g_aeopid[846].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(GOMER, lmRadDetect));
	g_aeopid[846].optdat.ibGet = static_cast<std::intptr_t>(offsetof(GOMER, lmRadDetect));
	g_aeopid[846].optdat.ibSet = static_cast<std::intptr_t>(offsetof(GOMER, lmRadDetect));

	g_aeopid[847].otyp = OTYP_Oid;
	g_aeopid[847].grfeopid = 128;
	g_aeopid[847].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(GOMER, oidDetect));

	g_aeopid[848].otyp = OTYP_Rail;
	g_aeopid[848].grfeopid = 18;
	g_aeopid[848].optdat.ibGet = static_cast<std::intptr_t>(offsetof(GOMER, pvolDetect));
	g_aeopid[848].optdat.ibSet = static_cast<std::intptr_t>(offsetof(GOMER, pvolDetect));

	g_aeopid[849].grfeopid = 0x92;
	g_aeopid[849].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(GOMER, fDetectLatchExternal));
	g_aeopid[849].otyp = OTYP_Bool;
	g_aeopid[849].optdat.ibGet = static_cast<std::intptr_t>(offsetof(GOMER, fDetectLatchExternal));
	g_aeopid[849].optdat.ibSet = static_cast<std::intptr_t>(offsetof(GOMER, fDetectLatchExternal));
	g_aeopid[850].grfeopid = 0x92;
	g_aeopid[850].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(GOMER, fAbandonExternal));
	g_aeopid[850].otyp = OTYP_Bool;
	g_aeopid[850].optdat.ibGet = static_cast<std::intptr_t>(offsetof(GOMER, fAbandonExternal));
	g_aeopid[850].optdat.ibSet = static_cast<std::intptr_t>(offsetof(GOMER, fAbandonExternal));

	g_aeopid[851].otyp = OTYP_Float;
	g_aeopid[851].grfeopid = 146;
	g_aeopid[851].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, dsCam));
	g_aeopid[851].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, dsCam));
	g_aeopid[851].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, dsCam));

	g_aeopid[852].otyp = OTYP_Clq;
	g_aeopid[852].grfeopid = 146;
	g_aeopid[852].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, aclqCam) + sizeof(CLQ) * 0);
	g_aeopid[852].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, aclqCam) + sizeof(CLQ) * 0);
	g_aeopid[852].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, aclqCam) + sizeof(CLQ) * 0);

	g_aeopid[853].otyp = OTYP_Lm;
	g_aeopid[853].grfeopid = 146;
	g_aeopid[853].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, almCam) + sizeof(LM) * 0);
	g_aeopid[853].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, almCam) + sizeof(LM) * 0);
	g_aeopid[853].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, almCam) + sizeof(LM) * 0);

	g_aeopid[854].otyp = OTYP_Clq;
	g_aeopid[854].grfeopid = 146;
	g_aeopid[854].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, aclqCam) + sizeof(CLQ) * 1);
	g_aeopid[854].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, aclqCam) + sizeof(CLQ) * 1);
	g_aeopid[854].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, aclqCam) + sizeof(CLQ) * 1);

	g_aeopid[855].otyp = OTYP_Lm;
	g_aeopid[855].grfeopid = 146;
	g_aeopid[855].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, almCam) + sizeof(LM) * 1);
	g_aeopid[855].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, almCam) + sizeof(LM) * 1);
	g_aeopid[855].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, almCam) + sizeof(LM) * 1);

	g_aeopid[856].otyp = OTYP_Clq;
	g_aeopid[856].grfeopid = 146;
	g_aeopid[856].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, aclqCam) + sizeof(CLQ) * 2);
	g_aeopid[856].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, aclqCam) + sizeof(CLQ) * 2);
	g_aeopid[856].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, aclqCam) + sizeof(CLQ) * 2);

	g_aeopid[857].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, almCam) + sizeof(LM) * 2);
	g_aeopid[857].otyp = OTYP_Lm;
	g_aeopid[857].grfeopid = 146;
	g_aeopid[857].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, almCam) + sizeof(LM) * 2);
	g_aeopid[857].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, almCam) + sizeof(LM) * 2);

	g_aeopid[858].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, dradSlack));
	g_aeopid[858].otyp = OTYP_Float;
	g_aeopid[858].grfeopid = 146;
	g_aeopid[858].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, dradSlack));
	g_aeopid[858].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, dradSlack));

	g_aeopid[859].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, xScreenPref));
	g_aeopid[859].otyp = OTYP_Float;
	g_aeopid[859].grfeopid = 146;
	g_aeopid[859].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, xScreenPref));
	g_aeopid[859].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, xScreenPref));

	g_aeopid[860].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, yScreenPref));
	g_aeopid[860].otyp = OTYP_Float;
	g_aeopid[860].grfeopid = 146;
	g_aeopid[860].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, yScreenPref));
	g_aeopid[860].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, yScreenPref));

	g_aeopid[861].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, uAdjust));
	g_aeopid[861].otyp = OTYP_Float;
	g_aeopid[861].grfeopid = 146;
	g_aeopid[861].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, uAdjust));
	g_aeopid[861].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, uAdjust));

	g_aeopid[862].otyp = OTYP_Float;
	g_aeopid[862].grfeopid = 146;
	g_aeopid[862].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, uFollow));
	g_aeopid[862].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, uFollow));
	g_aeopid[862].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, uFollow));

	g_aeopid[863].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, uLockOn));
	g_aeopid[863].otyp = OTYP_Float;
	g_aeopid[863].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, uLockOn));
	g_aeopid[863].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, uLockOn));
	g_aeopid[863].grfeopid = 146;

	g_aeopid[864].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetTnFocusRatio);
	g_aeopid[864].otyp = OTYP_Float;
	g_aeopid[864].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, vecFocus));
	g_aeopid[864].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetTnFocusRatio);
	g_aeopid[864].grfeopid = 290;

	g_aeopid[865].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, vecFocus));
	g_aeopid[865].otyp = OTYP_Vector;
	g_aeopid[865].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, vecFocus));
	g_aeopid[865].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, vecFocus));
	g_aeopid[865].grfeopid = 146;

	g_aeopid[866].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, fCylinder));
	g_aeopid[866].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, fCylinder));
	g_aeopid[866].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, fCylinder));
	g_aeopid[866].otyp = OTYP_Bool;
	g_aeopid[866].grfeopid = 146;

	g_aeopid[867].otyp = OTYP_Ftnd;
	g_aeopid[867].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, grftnd));
	g_aeopid[867].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, grftnd));
	g_aeopid[867].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, grftnd));
	g_aeopid[867].grfeopid = 146;

	g_aeopid[868].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, fPanOnEntry));
	g_aeopid[868].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, fPanOnEntry));
	g_aeopid[868].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, fPanOnEntry));
	g_aeopid[868].otyp = OTYP_Bool;
	g_aeopid[868].grfeopid = 146;

	g_aeopid[869].otyp = OTYP_Bool;
	g_aeopid[869].grfeopid = 146;
	g_aeopid[869].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, fPanOnIdle));
	g_aeopid[869].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, fPanOnIdle));
	g_aeopid[869].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, fPanOnIdle));

	g_aeopid[870].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, priCamera));
	g_aeopid[870].grfeopid = 146;
	g_aeopid[870].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, priCamera));
	g_aeopid[870].otyp = OTYP_Int;
	g_aeopid[870].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, priCamera));

	g_aeopid[871].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, fSwitchInAir));
	g_aeopid[871].grfeopid = 146;
	g_aeopid[871].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, fSwitchInAir));
	g_aeopid[871].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, fSwitchInAir));
	g_aeopid[871].otyp = OTYP_Bool;

	g_aeopid[872].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, cfk));
	g_aeopid[872].grfeopid = 146;
	g_aeopid[872].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, cfk));
	g_aeopid[872].otyp = OTYP_Dialogs;
	g_aeopid[872].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, cfk));

	g_aeopid[873].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, dtPanOnEntry));
	g_aeopid[873].grfeopid = 146;
	g_aeopid[873].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, dtPanOnEntry));
	g_aeopid[873].otyp = OTYP_Float;
	g_aeopid[873].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, dtPanOnEntry));

	g_aeopid[874].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, rswPanOnEntry));
	g_aeopid[874].grfeopid = 146;
	g_aeopid[874].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, rswPanOnEntry));
	g_aeopid[874].otyp = OTYP_Float;
	g_aeopid[874].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, rswPanOnEntry));

	g_aeopid[875].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, dtPanOnIdle));
	g_aeopid[875].grfeopid = 146;
	g_aeopid[875].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, dtPanOnIdle));
	g_aeopid[875].otyp = OTYP_Float;
	g_aeopid[875].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, dtPanOnIdle));

	g_aeopid[876].otyp = OTYP_Float;
	g_aeopid[876].grfeopid = 146;
	g_aeopid[876].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, rswPanOnIdle));
	g_aeopid[876].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, rswPanOnIdle));
	g_aeopid[876].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, rswPanOnIdle));

	g_aeopid[877].grfeopid = 146;
	g_aeopid[877].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, fCutOnActivate));
	g_aeopid[877].otyp = OTYP_Bool;
	g_aeopid[877].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, fCutOnActivate));
	g_aeopid[877].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, fCutOnActivate));

	g_aeopid[878].otyp = OTYP_Float;
	g_aeopid[878].grfeopid = 146;
	g_aeopid[878].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, radFOVOverride));
	g_aeopid[878].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, radFOVOverride));
	g_aeopid[878].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, radFOVOverride));

	g_aeopid[879].grfeopid = 146;
	g_aeopid[879].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, fNoSquish));
	g_aeopid[879].otyp = OTYP_Bool;
	g_aeopid[879].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, fNoSquish));
	g_aeopid[879].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, fNoSquish));

	g_aeopid[880].otyp = OTYP_Alo;
	g_aeopid[880].grfeopid = 18;
	g_aeopid[880].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, paloTarget));
	g_aeopid[880].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, paloTarget));

	g_aeopid[881].otyp = OTYP_Clq;
	g_aeopid[881].grfeopid = 146;
	g_aeopid[881].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, clqFollowTarget));
	g_aeopid[881].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, clqFollowTarget));
	g_aeopid[881].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, clqFollowTarget));

	g_aeopid[882].otyp = OTYP_Lm;
	g_aeopid[882].grfeopid = 146;
	g_aeopid[882].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, lmFollowTarget));
	g_aeopid[882].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, lmFollowTarget));
	g_aeopid[882].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, lmFollowTarget));

	g_aeopid[883].otyp = OTYP_Ack;
	g_aeopid[883].grfeopid = 2;
	g_aeopid[883].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tns));

	g_aeopid[884].otyp = OTYP_Ack;
	g_aeopid[884].grfeopid = 162;
	g_aeopid[884].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetTnTnsOverride);
	g_aeopid[884].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnsOverride));
	g_aeopid[884].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnsOverride));

	g_aeopid[885].otyp = OTYP_Float;
	g_aeopid[885].grfeopid = 146;
	g_aeopid[885].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, springStrength));
	g_aeopid[885].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, springStrength));
	g_aeopid[885].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, springStrength));

	g_aeopid[886].otyp = OTYP_Float;
	g_aeopid[886].grfeopid = 146;
	g_aeopid[886].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, springDamping));
	g_aeopid[886].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, springDamping));
	g_aeopid[886].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TN, tnfn) + offsetof(TNFN, springDamping));

	g_aeopid[887].grfeopid = 160;
	g_aeopid[887].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetJloStartJlovol);
	g_aeopid[887].otyp = OTYP_Oid;
	g_aeopid[887].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLO, oidJlovolStart));

	g_aeopid[888].otyp = OTYP_Void;
	g_aeopid[888].grfeopid = 1;
	g_aeopid[888].optdat.pvThunkFn = RefThunkJloACTIVATE;
	g_aeopid[888].optdat.ibSet = 0;
	g_aeopid[888].optdat.ibGet = 0;

	g_aeopid[889].otyp = OTYP_Void;
	g_aeopid[889].grfeopid = 1;
	g_aeopid[889].optdat.pvThunkFn = RefThunkJloDEACTIVATE;
	g_aeopid[889].optdat.ibSet = 0;
	g_aeopid[889].optdat.ibGet = 0;

	g_aeopid[890].grfeopid = 146;
	g_aeopid[890].otyp = OTYP_Bool;
	g_aeopid[890].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLO, fAmbientEnabled));
	g_aeopid[890].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLO, fAmbientEnabled));
	g_aeopid[890].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLO, fAmbientEnabled));

	g_aeopid[891].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLO, sStart));
	g_aeopid[891].otyp = OTYP_Float;
	g_aeopid[891].grfeopid = 146;
	g_aeopid[891].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLO, sStart));
	g_aeopid[891].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLO, sStart));

	g_aeopid[892].otyp = OTYP_Float;
	g_aeopid[892].grfeopid = 146;
	g_aeopid[892].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLO, sFull));
	g_aeopid[892].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLO, sFull));
	g_aeopid[892].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLO, sFull));

	g_aeopid[893].grfeopid = 146;
	g_aeopid[893].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLO, fDisableAutoSpin));
	g_aeopid[893].otyp = OTYP_Bool;
	g_aeopid[893].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLO, fDisableAutoSpin));
	g_aeopid[893].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLO, fDisableAutoSpin));

	g_aeopid[894].otyp = OTYP_Oid;
	g_aeopid[894].grfeopid = 128;
	g_aeopid[894].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLOVOL, oidLand));

	g_aeopid[895].otyp = OTYP_Warp;
	g_aeopid[895].grfeopid = 18;
	g_aeopid[895].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLOVOL, ppntLand));
	g_aeopid[895].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLOVOL, ppntLand));

	g_aeopid[896].otyp = OTYP_Oid;
	g_aeopid[896].grfeopid = 128;
	g_aeopid[896].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLOVOL, oidJloc));

	g_aeopid[897].otyp = OTYP_Tail;
	g_aeopid[897].grfeopid = 18;
	g_aeopid[897].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLOVOL, pjloc));
	g_aeopid[897].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLOVOL, pjloc));

	g_aeopid[898].otyp = OTYP_Iak;
	g_aeopid[898].grfeopid = 146;
	g_aeopid[898].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLOC, jlock));
	g_aeopid[898].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLOC, jlock));
	g_aeopid[898].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLOC, jlock));

	g_aeopid[899].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLOC, dtFire));
	g_aeopid[899].otyp = OTYP_Float;
	g_aeopid[899].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLOC, dtFire));
	g_aeopid[899].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLOC, dtFire));
	g_aeopid[899].grfeopid = 146;

	g_aeopid[900].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLOC, dtMissile));
	g_aeopid[900].otyp = OTYP_Float;
	g_aeopid[900].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLOC, dtMissile));
	g_aeopid[900].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLOC, dtMissile));
	g_aeopid[900].grfeopid = 146;

	g_aeopid[901].otyp = OTYP_Ropek;
	g_aeopid[901].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLOC, jlomk));
	g_aeopid[901].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLOC, jlomk));
	g_aeopid[901].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLOC, jlomk));
	g_aeopid[901].grfeopid = 146;

	g_aeopid[902].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLOC, rAccelTime));
	g_aeopid[902].otyp = OTYP_Float;
	g_aeopid[902].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLOC, rAccelTime));
	g_aeopid[902].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLOC, rAccelTime));
	g_aeopid[902].grfeopid = 146;

	g_aeopid[903].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLOC, rMissileSpeed));
	g_aeopid[903].otyp = OTYP_Float;
	g_aeopid[903].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLOC, rMissileSpeed));
	g_aeopid[903].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLOC, rMissileSpeed));
	g_aeopid[903].grfeopid = 146;

	g_aeopid[904].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLOC, radTilt));
	g_aeopid[904].otyp = OTYP_Float;
	g_aeopid[904].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLOC, radTilt));
	g_aeopid[904].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLOC, radTilt));
	g_aeopid[904].grfeopid = 146;

	g_aeopid[905].otyp = OTYP_Float;
	g_aeopid[905].grfeopid = 146;
	g_aeopid[905].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLOC, svLaunch));
	g_aeopid[905].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLOC, svLaunch));
	g_aeopid[905].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLOC, svLaunch));

	g_aeopid[906].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JLOC, svTarget));
	g_aeopid[906].otyp = OTYP_Float;
	g_aeopid[906].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JLOC, svTarget));
	g_aeopid[906].grfeopid = 146;
	g_aeopid[906].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JLOC, svTarget));

	g_aeopid[907].optdat.pvThunkFnUser = RefThunkRwmADD_AMMO;
	g_aeopid[907].otyp = OTYP_Void;
	g_aeopid[907].grfeopid = 1024;
	g_aeopid[907].optdat.ibSet = 4;
	g_aeopid[907].optdat.pvThunkFn = nullptr;

	g_aeopid[908].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWMCLONE, fSharedAmmo));
	g_aeopid[908].grfeopid = 4226;
	g_aeopid[908].otyp = OTYP_Bool;
	g_aeopid[908].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWMCLONE, fSharedAmmo));
	g_aeopid[908].optdat.pfnensure = (PFNENSURE)PrwmcloneEnsureRwm;

	g_aeopid[909].otyp = OTYP_Throbk;
	g_aeopid[909].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, cspk));
	g_aeopid[909].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, cspk));
	g_aeopid[909].grfeopid = 146;
	g_aeopid[909].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, cspk));

	g_aeopid[910].otyp = OTYP_Int;
	g_aeopid[910].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, irwc));
	g_aeopid[910].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, irwc));
	g_aeopid[910].grfeopid = 18;

	g_aeopid[911].optdat.pvThunkFn = RefThunkRwmENABLE_CACHE;
	g_aeopid[911].otyp = OTYP_Void;
	g_aeopid[911].optdat.ibSet = 1;
	g_aeopid[911].grfeopid = 1;
	g_aeopid[911].optdat.ibGet = 0;

	g_aeopid[912].otyp = OTYP_Void;
	g_aeopid[912].grfeopid = 1;
	g_aeopid[912].optdat.ibGet = 0;
	g_aeopid[912].optdat.ibSet = 1;
	g_aeopid[912].optdat.pvThunkFn = RefThunkRwmDISABLE_CACHE;

	g_aeopid[913].optdat.pvThunkFn = RefThunkRwmRESIZE_CACHE;
	g_aeopid[913].grfeopid = 1;
	g_aeopid[913].otyp = OTYP_Void;
	g_aeopid[913].optdat.ibGet = 0;
	g_aeopid[913].optdat.ibSet = 2;

	g_aeopid[914].grfeopid = 1;
	g_aeopid[914].otyp = OTYP_Void;
	g_aeopid[914].optdat.ibGet = 0;
	g_aeopid[914].optdat.ibSet = 0;
	g_aeopid[914].optdat.pvThunkFn = RefThunkRwmRELOAD;

	g_aeopid[915].grfeopid = 1;
	g_aeopid[915].otyp = OTYP_Bool;
	g_aeopid[915].optdat.ibGet = 0;
	g_aeopid[915].optdat.ibSet = 1;
	g_aeopid[915].optdat.pvThunkFn = RefThunkRwmFIRE;

	g_aeopid[916].grfeopid = 2;
	g_aeopid[916].otyp = OTYP_So;
	g_aeopid[916].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, psoAmmo));

	g_aeopid[917].grfeopid = 1;
	g_aeopid[917].otyp = OTYP_Void;
	g_aeopid[917].optdat.ibGet = 0;
	g_aeopid[917].optdat.ibSet = 0;
	g_aeopid[917].optdat.pvThunkFn = RefThunkRwmCLEAR_FIRE_INFO;

	g_aeopid[918].grfeopid = 128;
	g_aeopid[918].otyp = OTYP_Oid;
	g_aeopid[918].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, oidFire));

	g_aeopid[919].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, palo));
	g_aeopid[919].grfeopid = 18;
	g_aeopid[919].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, palo));
	g_aeopid[919].otyp = OTYP_Alo;

	g_aeopid[920].otyp = OTYP_Warp;
	g_aeopid[920].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, ppnt));
	g_aeopid[920].grfeopid = 18;
	g_aeopid[920].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, ppnt));

	g_aeopid[921].otyp = OTYP_Jackb;
	g_aeopid[921].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, pxfm));
	g_aeopid[921].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, pxfm));
	g_aeopid[921].grfeopid = 18;

	g_aeopid[922].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, pos));
	g_aeopid[922].otyp = OTYP_Vector;
	g_aeopid[922].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, pos));
	g_aeopid[922].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, pos));
	g_aeopid[922].grfeopid = 146;

	g_aeopid[923].otyp = OTYP_Matrix;
	g_aeopid[923].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, mat));
	g_aeopid[923].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, mat));
	g_aeopid[923].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, mat));
	g_aeopid[923].grfeopid = 146;

	g_aeopid[924].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, v));
	g_aeopid[924].otyp = OTYP_Vector;
	g_aeopid[924].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, v));
	g_aeopid[924].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, v));
	g_aeopid[924].grfeopid = 146;

	g_aeopid[925].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, w));
	g_aeopid[925].otyp = OTYP_Vector;
	g_aeopid[925].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, w));
	g_aeopid[925].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, w));
	g_aeopid[925].grfeopid = 146;

	g_aeopid[926].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, lmSw));
	g_aeopid[926].otyp = OTYP_Lm;
	g_aeopid[926].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, lmSw));
	g_aeopid[926].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, lmSw));
	g_aeopid[926].grfeopid = 146;

	g_aeopid[927].otyp = OTYP_Float;
	g_aeopid[927].grfeopid = 146;
	g_aeopid[927].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, tMax));
	g_aeopid[927].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, tMax));
	g_aeopid[927].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwfi) + offsetof(RWFI, tMax));

	g_aeopid[928].otyp = OTYP_Void;
	g_aeopid[928].grfeopid = 1;
	g_aeopid[928].optdat.ibSet = 0;
	g_aeopid[928].optdat.ibGet = 0;
	g_aeopid[928].optdat.pvThunkFn = RefThunkRwmCLEAR_TARGET_INFO;

	g_aeopid[929].otyp = OTYP_Oid;
	g_aeopid[929].grfeopid = 146;
	g_aeopid[929].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, oidTarget));
	g_aeopid[929].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, oidTarget));
	g_aeopid[929].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, oidTarget));

	g_aeopid[930].otyp = OTYP_Alo;
	g_aeopid[930].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, palo));
	g_aeopid[930].grfeopid = 18;
	g_aeopid[930].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, palo));

	g_aeopid[931].otyp = OTYP_Jackf;
	g_aeopid[931].grfeopid = 18;
	g_aeopid[931].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, ptarget));
	g_aeopid[931].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, ptarget));

	g_aeopid[932].otyp = OTYP_Vector;
	g_aeopid[932].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, pos));
	g_aeopid[932].grfeopid = 146;
	g_aeopid[932].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, pos));
	g_aeopid[932].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, pos));

	g_aeopid[933].otyp = OTYP_Dpk;
	g_aeopid[933].grfeopid = 146;
	g_aeopid[933].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, rwmtk));
	g_aeopid[933].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, rwmtk));
	g_aeopid[933].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, rwmtk));

	g_aeopid[934].otyp = OTYP_Clq;
	g_aeopid[934].grfeopid = 146;
	g_aeopid[934].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, clqSToDtPredict));
	g_aeopid[934].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, clqSToDtPredict));
	g_aeopid[934].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, clqSToDtPredict));

	g_aeopid[935].otyp = OTYP_Clq;
	g_aeopid[935].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, clqSToDtPredict));
	g_aeopid[935].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, clqSToDtPredict));
	g_aeopid[935].grfeopid = 146;
	g_aeopid[935].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, clqSToDtPredict));

	g_aeopid[936].otyp = OTYP_Lm;
	g_aeopid[936].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, lmDtPredict));
	g_aeopid[936].grfeopid = 146;
	g_aeopid[936].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, lmDtPredict));
	g_aeopid[936].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, lmDtPredict));

	g_aeopid[937].otyp = OTYP_Clq;
	g_aeopid[937].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, clqDistance));
	g_aeopid[937].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, clqDistance));
	g_aeopid[937].grfeopid = 146;
	g_aeopid[937].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, clqDistance));

	g_aeopid[938].otyp = OTYP_Lm;
	g_aeopid[938].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, lmDistance));
	g_aeopid[938].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, lmDistance));
	g_aeopid[938].grfeopid = 146;
	g_aeopid[938].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwti) + offsetof(RWTI, lmDistance));

	g_aeopid[939].otyp = OTYP_Void;
	g_aeopid[939].grfeopid = 1;
	g_aeopid[939].optdat.ibSet = 0;
	g_aeopid[939].optdat.pvThunkFn = RefThunkRwmCLEAR_AIM_CONSTRAINTS;
	g_aeopid[939].optdat.ibGet = 0;

	g_aeopid[940].otyp = OTYP_Flyk;
	g_aeopid[940].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwac) + offsetof(RWAC, grfask));
	g_aeopid[940].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwac) + offsetof(RWAC, grfask));
	g_aeopid[940].grfeopid = 146;
	g_aeopid[940].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwac) + offsetof(RWAC, grfask));

	g_aeopid[941].otyp = OTYP_Oid;
	g_aeopid[941].grfeopid = 128;
	g_aeopid[941].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, oidAimXfm));

	g_aeopid[942].otyp = OTYP_Jackb;
	g_aeopid[942].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwac) + offsetof(RWAC, pxfm));
	g_aeopid[942].grfeopid = 18;
	g_aeopid[942].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwac) + offsetof(RWAC, pxfm));

	g_aeopid[943].otyp = OTYP_Float;
	g_aeopid[943].grfeopid = 146;
	g_aeopid[943].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwac) + offsetof(RWAC, radTilt));
	g_aeopid[943].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwac) + offsetof(RWAC, radTilt));
	g_aeopid[943].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwac) + offsetof(RWAC, radTilt));

	g_aeopid[944].otyp = OTYP_Float;
	g_aeopid[944].grfeopid = 146;
	g_aeopid[944].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwac) + offsetof(RWAC, radPan));
	g_aeopid[944].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwac) + offsetof(RWAC, radPan));
	g_aeopid[944].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwac) + offsetof(RWAC, radPan));

	g_aeopid[945].otyp = OTYP_Float;
	g_aeopid[945].grfeopid = 146;
	g_aeopid[945].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, rwac) + offsetof(RWAC, sv));
	g_aeopid[945].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, rwac) + offsetof(RWAC, sv));
	g_aeopid[945].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RWM, rwac) + offsetof(RWAC, sv));

	g_aeopid[946].otyp = OTYP_Oid;
	g_aeopid[946].grfeopid = 128;
	g_aeopid[946].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RWM, oidAim));

	g_aeopid[947].otyp = OTYP_Alo;
	g_aeopid[947].grfeopid = 2;
	g_aeopid[947].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RWM, paloFireContext));

	g_aeopid[948].otyp = OTYP_Fexplso;
	g_aeopid[948].grfeopid = 2178;
	g_aeopid[948].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, flyk));
	g_aeopid[948].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, flyk));
	g_aeopid[948].optdat.fDef = 0;

	g_aeopid[949].otyp = OTYP_Cspk;
	g_aeopid[949].grfeopid = 2178;
	g_aeopid[949].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, flysInit));
	g_aeopid[949].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, flysInit));
	g_aeopid[949].optdat.fDef = 0;

	g_aeopid[950].otyp = OTYP_Cspk;
	g_aeopid[950].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, flys));
	g_aeopid[950].grfeopid = 34;
	g_aeopid[950].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetFlyFlys);

	g_aeopid[951].otyp = OTYP_Lm;
	g_aeopid[951].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, lmSCylinderRadius));
	g_aeopid[951].grfeopid = 146;
	g_aeopid[951].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, lmSCylinderRadius));
	g_aeopid[951].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FLY, lmSCylinderRadius));

	g_aeopid[952].otyp = OTYP_Lm;
	g_aeopid[952].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, lmSCylinderHeight));
	g_aeopid[952].grfeopid = 146;
	g_aeopid[952].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, lmSCylinderHeight));
	g_aeopid[952].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FLY, lmSCylinderHeight));

	g_aeopid[953].otyp = OTYP_Lm;
	g_aeopid[953].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, lmSvxyFlight));
	g_aeopid[953].grfeopid = 146;
	g_aeopid[953].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, lmSvxyFlight));
	g_aeopid[953].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FLY, lmSvxyFlight));

	g_aeopid[954].otyp = OTYP_Lm;
	g_aeopid[954].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, lmSvzFlight));
	g_aeopid[954].grfeopid = 146;
	g_aeopid[954].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, lmSvzFlight));
	g_aeopid[954].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FLY, lmSvzFlight));

	g_aeopid[955].otyp = OTYP_Oid;
	g_aeopid[955].grfeopid = 128;
	g_aeopid[955].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, oidSoar));

	g_aeopid[956].otyp = OTYP_Float;
	g_aeopid[956].grfeopid = 2194;
	g_aeopid[956].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, uSoar));
	g_aeopid[956].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FLY, uSoar));
	g_aeopid[956].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, uSoar));
	g_aeopid[956].optdat.fDef = 1061997773;

	g_aeopid[957].otyp = OTYP_Lm;
	g_aeopid[957].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, lmDtFlying));
	g_aeopid[957].grfeopid = 146;
	g_aeopid[957].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, lmDtFlying));
	g_aeopid[957].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FLY, lmDtFlying));

	g_aeopid[958].otyp = OTYP_Float;
	g_aeopid[958].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, dtFlyingFrame));
	g_aeopid[958].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, dtFlyingFrame));
	g_aeopid[958].grfeopid = 2194;
	g_aeopid[958].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FLY, dtFlyingFrame));
	g_aeopid[958].optdat.fDef = 1017370378;

	g_aeopid[959].otyp = OTYP_Lm;
	g_aeopid[959].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, lmDtGround));
	g_aeopid[959].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, lmDtGround));
	g_aeopid[959].grfeopid = 146;
	g_aeopid[959].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FLY, lmDtGround));

	g_aeopid[960].otyp = OTYP_Lm;
	g_aeopid[960].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, lmDtGroundFrame));
	g_aeopid[960].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, lmDtGroundFrame));
	g_aeopid[960].grfeopid = 146;
	g_aeopid[960].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FLY, lmDtGroundFrame));

	g_aeopid[961].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, lmDtGroundFramePause));
	g_aeopid[961].otyp = OTYP_Lm;
	g_aeopid[961].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, lmDtGroundFramePause));
	g_aeopid[961].grfeopid = 146;
	g_aeopid[961].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FLY, lmDtGroundFramePause));

	g_aeopid[962].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, lmDtDroppings));
	g_aeopid[962].otyp = OTYP_Lm;
	g_aeopid[962].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, lmDtDroppings));
	g_aeopid[962].grfeopid = 146;
	g_aeopid[962].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FLY, lmDtDroppings));

	g_aeopid[963].otyp = OTYP_Float;
	g_aeopid[963].grfeopid = 2194;
	g_aeopid[963].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, sFlee));
	g_aeopid[963].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, sFlee));
	g_aeopid[963].optdat.fDef = 1133903872;
	g_aeopid[963].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FLY, sFlee));

	g_aeopid[964].otyp = OTYP_Int;
	g_aeopid[964].grfeopid = 146;
	g_aeopid[964].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, ccoin));
	g_aeopid[964].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, ccoin));
	g_aeopid[964].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FLY, ccoin));

	g_aeopid[965].grfeopid = 146;
	g_aeopid[965].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(FLY, fSuppressDifficultyUpdate));
	g_aeopid[965].otyp = OTYP_Bool;
	g_aeopid[965].optdat.ibGet = static_cast<std::intptr_t>(offsetof(FLY, fSuppressDifficultyUpdate));
	g_aeopid[965].optdat.ibSet = static_cast<std::intptr_t>(offsetof(FLY, fSuppressDifficultyUpdate));

	g_aeopid[966].otyp = OTYP_Dprizes;
	g_aeopid[966].grfeopid = 34;
	g_aeopid[966].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetDialogDialogs);
	g_aeopid[966].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DIALOG, dialogs));

	g_aeopid[967].otyp = OTYP_Oid;
	g_aeopid[967].grfeopid = 128;
	g_aeopid[967].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DIALOG, oidPntAnchorDefault));

	g_aeopid[968].grfeopid = 146;
	g_aeopid[968].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DIALOG, fDefault));
	g_aeopid[968].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DIALOG, fDefault));
	g_aeopid[968].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DIALOG, fDefault));
	g_aeopid[968].otyp = OTYP_Bool;

	g_aeopid[969].grfeopid = 146;
	g_aeopid[969].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DIALOG, fCrucial));
	g_aeopid[969].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DIALOG, fCrucial));
	g_aeopid[969].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DIALOG, fCrucial));
	g_aeopid[969].otyp = OTYP_Bool;

	g_aeopid[970].grfeopid = 146;
	g_aeopid[970].otyp = OTYP_Bool;
	g_aeopid[970].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DIALOG, fNoSkip));
	g_aeopid[970].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DIALOG, fNoSkip));
	g_aeopid[970].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DIALOG, fNoSkip));

	g_aeopid[971].optdat.pvThunkFnUser = RefThunkDialogINSTRUCT;
	g_aeopid[971].otyp = OTYP_Void;
	g_aeopid[971].grfeopid = 1024;
	g_aeopid[971].optdat.ibSet = 0;
	g_aeopid[971].optdat.pvThunkFn = nullptr;

	g_aeopid[972].optdat.pvThunkFnUser = RefThunkDialogCONFRONT;
	g_aeopid[972].otyp = OTYP_Void;
	g_aeopid[972].grfeopid = 1024;
	g_aeopid[972].optdat.ibSet = 0;
	g_aeopid[972].optdat.pvThunkFn = nullptr;

	g_aeopid[973].optdat.pvThunkFnUser = RefThunkDialogADD_EQUIVALENCE;
	g_aeopid[973].otyp = OTYP_Void;
	g_aeopid[973].grfeopid = 1024;
	g_aeopid[973].optdat.ibSet = 2;
	g_aeopid[973].optdat.pvThunkFn = nullptr;

	g_aeopid[974].otyp = OTYP_Void;
	g_aeopid[974].grfeopid = 1;
	g_aeopid[974].optdat.ibGet = 0;
	g_aeopid[974].optdat.ibSet = 0;
	g_aeopid[974].optdat.pvThunkFn = RefThunkDialogTRIGGER;

	g_aeopid[975].otyp = OTYP_Void;
	g_aeopid[975].grfeopid = 1;
	g_aeopid[975].optdat.ibGet = 0;
	g_aeopid[975].optdat.ibSet = 0;
	g_aeopid[975].optdat.pvThunkFn = RefThunkDialogUNTRIGGER;

	g_aeopid[976].otyp = OTYP_Oid;
	g_aeopid[976].grfeopid = 146;
	g_aeopid[976].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DIALOG, oidPntAnchor));
	g_aeopid[976].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DIALOG, oidPntAnchor));
	g_aeopid[976].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DIALOG, oidPntAnchor));

	g_aeopid[977].otyp = OTYP_Bool;
	g_aeopid[977].grfeopid = 36;
	g_aeopid[977].optdat.pvThunkFnUser = (PVTHUNKFN)GetDialogPlayed;
	g_aeopid[977].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetDialogPlayed);

	g_aeopid[978].grfeopid = 130;
	g_aeopid[978].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DIALOG, fConfrontAudio));
	g_aeopid[978].otyp = OTYP_Bool;
	g_aeopid[978].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DIALOG, fConfrontAudio));

	g_aeopid[979].otyp = OTYP_Oid;
	g_aeopid[979].grfeopid = 146;
	g_aeopid[979].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DPRIZE, oidInitialState));
	g_aeopid[979].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DPRIZE, oidInitialState));
	g_aeopid[979].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DPRIZE, oidInitialState));

	g_aeopid[980].otyp = OTYP_Float;
	g_aeopid[980].grfeopid = 146;
	g_aeopid[980].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DPRIZE, dtInitialSkip));
	g_aeopid[980].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DPRIZE, dtInitialSkip));
	g_aeopid[980].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DPRIZE, dtInitialSkip));

	g_aeopid[981].otyp = OTYP_Dprizes;
	g_aeopid[981].grfeopid = 66;
	g_aeopid[981].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DPRIZE, dprizes));
	g_aeopid[981].optdat.ibSet = static_cast<std::intptr_t>(offsetof(VTDPRIZE, pfnSetDprizeDprizes));

	g_aeopid[982].grfeopid = 146;
	g_aeopid[982].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DPRIZE, fNeverReuse));
	g_aeopid[982].otyp = OTYP_Bool;
	g_aeopid[982].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DPRIZE, fNeverReuse));
	g_aeopid[982].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DPRIZE, fNeverReuse));

	g_aeopid[983].otyp = OTYP_Float;
	g_aeopid[983].grfeopid = 146;
	g_aeopid[983].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DPRIZE, svcAttract));
	g_aeopid[983].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DPRIZE, svcAttract));
	g_aeopid[983].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DPRIZE, svcAttract));

	g_aeopid[984].grfeopid = 146;
	g_aeopid[984].otyp = OTYP_Bool;
	g_aeopid[984].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DPRIZE, fCollectible));
	g_aeopid[984].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DPRIZE, fCollectible));
	g_aeopid[984].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(DPRIZE, fCollectible));

	g_aeopid[985].otyp = OTYP_Lm;
	g_aeopid[985].grfeopid = 146;
	g_aeopid[985].optdat.ibGet = static_cast<std::intptr_t>(offsetof(COIN, lmDtMaxLifetime));
	g_aeopid[985].optdat.ibSet = static_cast<std::intptr_t>(offsetof(COIN, lmDtMaxLifetime));
	g_aeopid[985].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(COIN, lmDtMaxLifetime));

	g_aeopid[986].grfeopid = 146;
	g_aeopid[986].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(LIFETKN, lmDtReuse));
	g_aeopid[986].otyp = OTYP_Lm;
	g_aeopid[986].optdat.ibSet = static_cast<std::intptr_t>(offsetof(LIFETKN, lmDtReuse));
	g_aeopid[986].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LIFETKN, lmDtReuse));

	g_aeopid[987].grfeopid = 0;
	g_aeopid[987].otyp = OTYP_Bool;

	g_aeopid[988].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPEAKER, rgbaTextPacked));
	g_aeopid[988].grfeopid = 146;
	g_aeopid[988].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SPEAKER, rgbaTextPacked));
	g_aeopid[988].otyp = OTYP_Int;
	g_aeopid[988].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SPEAKER, rgbaTextPacked));

	g_aeopid[989].grfeopid = 146;
	g_aeopid[989].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SPEAKER, radFov));
	g_aeopid[989].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPEAKER, radFov));
	g_aeopid[989].otyp = OTYP_Float;
	g_aeopid[989].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SPEAKER, radFov));

	g_aeopid[990].grfeopid = 146;
	g_aeopid[990].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SPEAKER, sDistCm));
	g_aeopid[990].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPEAKER, sDistCm));
	g_aeopid[990].otyp = OTYP_Float;
	g_aeopid[990].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SPEAKER, sDistCm));

	g_aeopid[991].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPEAKER, uxTv));
	g_aeopid[991].grfeopid = 146;
	g_aeopid[991].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SPEAKER, uxTv));
	g_aeopid[991].otyp = OTYP_Float;
	g_aeopid[991].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SPEAKER, uxTv));

	g_aeopid[992].otyp = OTYP_Float;
	g_aeopid[992].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPEAKER, uyTv));
	g_aeopid[992].grfeopid = 146;
	g_aeopid[992].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SPEAKER, uyTv));
	g_aeopid[992].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SPEAKER, uyTv));

	g_aeopid[993].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPEAKER, dposLight));
	g_aeopid[993].grfeopid = 146;
	g_aeopid[993].otyp = OTYP_Vector;
	g_aeopid[993].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SPEAKER, dposLight));
	g_aeopid[993].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SPEAKER, dposLight));

	g_aeopid[994].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPEAKER, dposLightConfront));
	g_aeopid[994].grfeopid = 146;
	g_aeopid[994].otyp = OTYP_Vector;
	g_aeopid[994].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SPEAKER, dposLightConfront));
	g_aeopid[994].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SPEAKER, dposLightConfront));

	g_aeopid[995].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPEAKER, uxTvConfront));
	g_aeopid[995].grfeopid = 146;
	g_aeopid[995].otyp = OTYP_Float;
	g_aeopid[995].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SPEAKER, uxTvConfront));
	g_aeopid[995].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SPEAKER, uxTvConfront));

	g_aeopid[996].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPEAKER, uyTvConfront));
	g_aeopid[996].grfeopid = 146;
	g_aeopid[996].otyp = OTYP_Float;
	g_aeopid[996].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SPEAKER, uyTvConfront));
	g_aeopid[996].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SPEAKER, uyTvConfront));

	g_aeopid[997].grfeopid = 160;
	g_aeopid[997].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPEAKER, oidSmIdle));
	g_aeopid[997].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSpeakerSmIdle);
	g_aeopid[997].otyp = OTYP_Oid;

	g_aeopid[998].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPEAKER, dposTvConfrontCamera));
	g_aeopid[998].grfeopid = 146;
	g_aeopid[998].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SPEAKER, dposTvConfrontCamera));
	g_aeopid[998].otyp = OTYP_Vector;
	g_aeopid[998].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SPEAKER, dposTvConfrontCamera));

	g_aeopid[999].otyp = OTYP_Float;
	g_aeopid[999].grfeopid = 146;
	g_aeopid[999].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SPEAKER, radTvConfrontRoll));
	g_aeopid[999].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPEAKER, radTvConfrontRoll));
	g_aeopid[999].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SPEAKER, radTvConfrontRoll));

	g_aeopid[1000].otyp = OTYP_Oid;
	g_aeopid[1000].grfeopid = 256;
	g_aeopid[1000].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddLockgLock);

	g_aeopid[1001].otyp = OTYP_Void;
	g_aeopid[1001].grfeopid = 1;
	g_aeopid[1001].optdat.ibSet = 0;
	g_aeopid[1001].optdat.pvThunkFn = RefThunkLockgTRIGGER;
	g_aeopid[1001].optdat.ibGet = 0;

	g_aeopid[1002].otyp = OTYP_Int;
	g_aeopid[1002].grfeopid = 256;
	g_aeopid[1002].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetLockgIndex);

	g_aeopid[1003].otyp = OTYP_Void;
	g_aeopid[1003].grfeopid = 1024;
	g_aeopid[1003].optdat.pvThunkFnUser = RefThunkWrADD_CIRCLE_WARP;
	g_aeopid[1003].optdat.ibSet = 0;
	g_aeopid[1003].optdat.pvThunkFn = nullptr;

	g_aeopid[1004].otyp = OTYP_Void;
	g_aeopid[1004].grfeopid = 1024;
	g_aeopid[1004].optdat.pvThunkFnUser = RefThunkWrADD_BEND_WARP;
	g_aeopid[1004].optdat.ibSet = 0;
	g_aeopid[1004].optdat.pvThunkFn = nullptr;

	g_aeopid[1005].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, normal));
	g_aeopid[1005].otyp = OTYP_Vector;
	g_aeopid[1005].grfeopid = 4242;
	g_aeopid[1005].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, normal));
	g_aeopid[1005].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, normal));
	g_aeopid[1005].optdat.pfnensure = (PFNENSURE)PwreGetWrCircle;

	g_aeopid[1006].grfeopid = 292;
	g_aeopid[1006].otyp = OTYP_Float;
	g_aeopid[1006].optdat.pvThunkFnUser = (PVTHUNKFN)GetWrWavelength;
	g_aeopid[1006].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetWrWavelength);
	g_aeopid[1006].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetWrWavelength);

	g_aeopid[1007].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, gFrequency));
	g_aeopid[1007].otyp = OTYP_Float;
	g_aeopid[1007].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, gFrequency));
	g_aeopid[1007].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, gFrequency));
	g_aeopid[1007].optdat.pfnensure = (PFNENSURE)PwreGetWrCircle;
	g_aeopid[1007].grfeopid = 4242;

	g_aeopid[1008].otyp = OTYP_Float;
	g_aeopid[1008].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, uPhase));
	g_aeopid[1008].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, uPhase));
	g_aeopid[1008].optdat.pfnensure = (PFNENSURE)PwreGetWrCircle;
	g_aeopid[1008].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, uPhase));
	g_aeopid[1008].grfeopid = 4242;

	g_aeopid[1009].otyp = OTYP_Vector;
	g_aeopid[1009].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, awref) + offsetof(WREF, dpos));
	g_aeopid[1009].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, awref) + offsetof(WREF, dpos));
	g_aeopid[1009].optdat.pfnensure = (PFNENSURE)PwreGetWrCircle;
	g_aeopid[1009].grfeopid = 4242;
	g_aeopid[1009].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, awref) + offsetof(WREF, dpos));

	g_aeopid[1010].otyp = OTYP_Vector;
	g_aeopid[1010].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, awref) + sizeof(WREF) + offsetof(WREF, dpos));
	g_aeopid[1010].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, awref) + sizeof(WREF) + offsetof(WREF, dpos));
	g_aeopid[1010].optdat.pfnensure = (PFNENSURE)PwreGetWrCircle;
	g_aeopid[1010].grfeopid = 4242;
	g_aeopid[1010].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, awref) + sizeof(WREF) + offsetof(WREF, dpos));

	g_aeopid[1011].otyp = OTYP_Lm;
	g_aeopid[1011].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, awref) + offsetof(WREF, duv));
	g_aeopid[1011].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, awref) + offsetof(WREF, duv));
	g_aeopid[1011].optdat.pfnensure = (PFNENSURE)PwreGetWrCircle;
	g_aeopid[1011].grfeopid = 4242;
	g_aeopid[1011].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, awref) + offsetof(WREF, duv));

	g_aeopid[1012].otyp = OTYP_Lm;
	g_aeopid[1012].grfeopid = 4242;
	g_aeopid[1012].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, awref) + sizeof(WREF) + offsetof(WREF, duv));
	g_aeopid[1012].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, awref) + sizeof(WREF) + offsetof(WREF, duv));
	g_aeopid[1012].optdat.pfnensure = (PFNENSURE)PwreGetWrCircle;
	g_aeopid[1012].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WRE, circle) + offsetof(WRE::CIRCLE, awref) + sizeof(WREF) + offsetof(WREF, duv));

	g_aeopid[1013].grfeopid = 4242;
	g_aeopid[1013].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, normalBend));
	g_aeopid[1013].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, normalBend));
	g_aeopid[1013].otyp = OTYP_Vector;
	g_aeopid[1013].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, normalBend));
	g_aeopid[1013].optdat.pfnensure = (PFNENSURE)PwreGetWrBend;

	g_aeopid[1014].grfeopid = 4242;
	g_aeopid[1014].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, onzRadBend) + offsetof(ONZ, lm));
	g_aeopid[1014].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, onzRadBend) + offsetof(ONZ, lm));
	g_aeopid[1014].otyp = OTYP_Lm;
	g_aeopid[1014].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, onzRadBend) + offsetof(ONZ, lm));
	g_aeopid[1014].optdat.pfnensure = (PFNENSURE)PwreGetWrBend;

	g_aeopid[1015].grfeopid = 4242;
	g_aeopid[1015].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, onzRadBend) + offsetof(ONZ, uBias));
	g_aeopid[1015].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, onzRadBend) + offsetof(ONZ, uBias));
	g_aeopid[1015].otyp = OTYP_Float;
	g_aeopid[1015].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, onzRadBend) + offsetof(ONZ, uBias));
	g_aeopid[1015].optdat.pfnensure = (PFNENSURE)PwreGetWrBend;

	g_aeopid[1016].optdat.pvThunkFnUser = RefThunkWrADD_BEND_NOISE;
	g_aeopid[1016].grfeopid = 1024;
	g_aeopid[1016].optdat.ibSet = 4;
	g_aeopid[1016].otyp = OTYP_Void;
	g_aeopid[1016].optdat.pvThunkFn = nullptr;

	g_aeopid[1017].grfeopid = 4242;
	g_aeopid[1017].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, normalSwivel));
	g_aeopid[1017].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, normalSwivel));
	g_aeopid[1017].otyp = OTYP_Vector;
	g_aeopid[1017].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, normalSwivel));
	g_aeopid[1017].optdat.pfnensure = (PFNENSURE)PwreGetWrBend;

	g_aeopid[1018].otyp = OTYP_Lm;
	g_aeopid[1018].grfeopid = 4242;
	g_aeopid[1018].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, onzRadSwivel) + offsetof(ONZ, lm));
	g_aeopid[1018].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, onzRadSwivel) + offsetof(ONZ, lm));
	g_aeopid[1018].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, onzRadSwivel) + offsetof(ONZ, lm));
	g_aeopid[1018].optdat.pfnensure = (PFNENSURE)PwreGetWrBend;

	g_aeopid[1019].otyp = OTYP_Float;
	g_aeopid[1019].grfeopid = 4242;
	g_aeopid[1019].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, onzRadSwivel) + offsetof(ONZ, uBias));
	g_aeopid[1019].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, onzRadSwivel) + offsetof(ONZ, uBias));
	g_aeopid[1019].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(WRE, bend) + offsetof(WRE::BEND, onzRadSwivel) + offsetof(ONZ, uBias));
	g_aeopid[1019].optdat.pfnensure = (PFNENSURE)PwreGetWrBend;

	g_aeopid[1020].otyp = OTYP_Void;
	g_aeopid[1020].grfeopid = 1024;
	g_aeopid[1020].optdat.pvThunkFnUser = RefThunkWrADD_SWIVEL_NOISE;
	g_aeopid[1020].optdat.ibSet = 4;
	g_aeopid[1020].optdat.pvThunkFn = nullptr;

	g_aeopid[1021].otyp = OTYP_Int;
	g_aeopid[1021].grfeopid = 2;
	g_aeopid[1021].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WR, cwre));

	g_aeopid[1022].otyp = OTYP_Int;
	g_aeopid[1022].grfeopid = 18;
	g_aeopid[1022].optdat.ibSet = static_cast<std::intptr_t>(offsetof(WR, iwreCur));
	g_aeopid[1022].optdat.ibGet = static_cast<std::intptr_t>(offsetof(WR, iwreCur));

	g_aeopid[1023].grfeopid = 146;
	g_aeopid[1023].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(MBG, fAbandonExternal));
	g_aeopid[1023].otyp = OTYP_Bool;
	g_aeopid[1023].optdat.ibGet = static_cast<std::intptr_t>(offsetof(MBG, fAbandonExternal));
	g_aeopid[1023].optdat.ibSet = static_cast<std::intptr_t>(offsetof(MBG, fAbandonExternal));

	g_aeopid[1024].otyp = OTYP_Map;
	g_aeopid[1024].grfeopid = 2;
	g_aeopid[1024].optdat.ibGet = static_cast<std::intptr_t>(offsetof(MBG, psmaMbg));

	g_aeopid[1025].otyp = OTYP_Jackb;
	g_aeopid[1025].grfeopid = 146;
	g_aeopid[1025].optdat.ibGet = static_cast<std::intptr_t>(offsetof(MBG, pxfm));
	g_aeopid[1025].optdat.ibSet = static_cast<std::intptr_t>(offsetof(MBG, pxfm));
	g_aeopid[1025].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(MBG, pxfm));

	g_aeopid[1026].optdat.pvThunkFn = RefThunkMbgUPDATE_AI;
	g_aeopid[1026].otyp = OTYP_Void;
	g_aeopid[1026].grfeopid = 1;
	g_aeopid[1026].optdat.ibGet = 0;
	g_aeopid[1026].optdat.ibSet = 0;

	g_aeopid[1027].otyp = OTYP_Float;
	g_aeopid[1027].grfeopid = 4;
	g_aeopid[1027].optdat.pvThunkFnUser = (PVTHUNKFN)GetMbgAttackBlend;

	g_aeopid[1028].otyp = OTYP_Lookk;
	g_aeopid[1028].grfeopid = 34;
	g_aeopid[1028].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TANK, tanks));
	g_aeopid[1028].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetTankTanks);

	g_aeopid[1029].grfeopid = 146;
	g_aeopid[1029].otyp = OTYP_Bool;
	g_aeopid[1029].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TANK, fAllowEject));
	g_aeopid[1029].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TANK, fAllowEject));
	g_aeopid[1029].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TANK, fAllowEject));

	g_aeopid[1030].otyp = OTYP_Float;
	g_aeopid[1030].grfeopid = 146;
	g_aeopid[1030].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SPIRE, dzJumpTargetMax));
	g_aeopid[1030].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SPIRE, dzJumpTargetMax));
	g_aeopid[1030].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPIRE, dzJumpTargetMax));

	g_aeopid[1031].grfeopid = 146;
	g_aeopid[1031].otyp = OTYP_Bool;
	g_aeopid[1031].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SPIRE, unkInt));
	g_aeopid[1031].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SPIRE, unkInt));
	g_aeopid[1031].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SPIRE, unkInt));

	g_aeopid[1032].grfeopid = 146;
	g_aeopid[1032].otyp = OTYP_Bool;
	g_aeopid[1032].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAIL, fSlippery));
	g_aeopid[1032].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAIL, fSlippery));
	g_aeopid[1032].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAIL, fSlippery));

	g_aeopid[1033].otyp = OTYP_Float;
	g_aeopid[1033].grfeopid = 2194;
	g_aeopid[1033].optdat.fDef = 1148846080;
	g_aeopid[1033].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAIL, svMax));
	g_aeopid[1033].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAIL, svMax));
	g_aeopid[1033].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAIL, svMax));

	g_aeopid[1034].otyp = OTYP_Float;
	g_aeopid[1034].grfeopid = 2194;
	g_aeopid[1034].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAIL, rdvGravity));
	g_aeopid[1034].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAIL, rdvGravity));
	g_aeopid[1034].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAIL, rdvGravity));
	g_aeopid[1034].optdat.fDef = 1073741824;

	g_aeopid[1035].otyp = OTYP_Float;
	g_aeopid[1035].grfeopid = 146;
	g_aeopid[1035].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAIL, dzJumpTargetMax));
	g_aeopid[1035].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAIL, dzJumpTargetMax));
	g_aeopid[1035].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAIL, dzJumpTargetMax));

	g_aeopid[1036].otyp = OTYP_Bool;
	g_aeopid[1036].grfeopid = 146;
	g_aeopid[1036].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAIL, unkInt));
	g_aeopid[1036].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAIL, unkInt));
	g_aeopid[1036].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAIL, unkInt));

	g_aeopid[1037].otyp = OTYP_Oid;
	g_aeopid[1037].grfeopid = 128;
	g_aeopid[1037].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(RAIL, oidRope));

	g_aeopid[1038].otyp = OTYP_Rope;
	g_aeopid[1038].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RAIL, prope));
	g_aeopid[1038].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RAIL, prope));
	g_aeopid[1038].grfeopid = 18;

	g_aeopid[1039].otyp = OTYP_Map;
	g_aeopid[1039].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TURRET, psmaTurret));
	g_aeopid[1039].grfeopid = 2;

	g_aeopid[1040].otyp = OTYP_Bool;
	g_aeopid[1040].grfeopid = 146;
	g_aeopid[1040].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TURRET, fSetCameraMatrix));
	g_aeopid[1040].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TURRET, fSetCameraMatrix));
	g_aeopid[1040].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TURRET, fSetCameraMatrix));

	g_aeopid[1041].otyp = OTYP_Wek;
	g_aeopid[1041].grfeopid = 146;
	g_aeopid[1041].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TURRET, lookk));
	g_aeopid[1041].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TURRET, lookk));
	g_aeopid[1041].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TURRET, lookk));

	g_aeopid[1042].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TURRET, fTrack));
	g_aeopid[1042].grfeopid = 0x92;
	g_aeopid[1042].otyp = OTYP_Bool;
	g_aeopid[1042].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TURRET, fTrack));
	g_aeopid[1042].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TURRET, fTrack));

	g_aeopid[1043].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddTurretTargetOid);
	g_aeopid[1043].otyp = OTYP_Oid;
	g_aeopid[1043].grfeopid = 256;

	g_aeopid[1044].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TURRET, lmTilt));
	g_aeopid[1044].otyp = OTYP_Lm;
	g_aeopid[1044].grfeopid = 146;
	g_aeopid[1044].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TURRET, lmTilt));
	g_aeopid[1044].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TURRET, lmTilt));

	g_aeopid[1045].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(TURRET, lmPan));
	g_aeopid[1045].otyp = OTYP_Lm;
	g_aeopid[1045].grfeopid = 146;
	g_aeopid[1045].optdat.ibSet = static_cast<std::intptr_t>(offsetof(TURRET, lmPan));
	g_aeopid[1045].optdat.ibGet = static_cast<std::intptr_t>(offsetof(TURRET, lmPan));

	g_aeopid[1046].otyp = OTYP_Int;
	g_aeopid[1046].grfeopid = 146;
	g_aeopid[1046].optdat.ibSet = static_cast<std::intptr_t>(offsetof(VAULT, nCombination));
	g_aeopid[1046].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(VAULT, nCombination));
	g_aeopid[1046].optdat.ibGet = static_cast<std::intptr_t>(offsetof(VAULT, nCombination));

	g_aeopid[1047].otyp = OTYP_Void;
	g_aeopid[1047].grfeopid = 1024;
	g_aeopid[1047].optdat.pvThunkFnUser = RefThunkVaultADD_GOAD_DIALOG;
	g_aeopid[1047].optdat.ibSet = 2;
	g_aeopid[1047].optdat.pvThunkFn = nullptr;

	g_aeopid[1048].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(VAULT, oidVolbtnGoad));
	g_aeopid[1048].otyp = OTYP_Oid;
	g_aeopid[1048].grfeopid = 128;

	g_aeopid[1049].grfeopid = 128;
	g_aeopid[1049].otyp = OTYP_Oid;
	g_aeopid[1049].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(VAULT, oidDialogCombo));

	g_aeopid[1050].grfeopid = 146;
	g_aeopid[1050].otyp = OTYP_Bool;
	g_aeopid[1050].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(VAULT, fGoadStart));
	g_aeopid[1050].optdat.ibGet = static_cast<std::intptr_t>(offsetof(VAULT, fGoadStart));
	g_aeopid[1050].optdat.ibSet = static_cast<std::intptr_t>(offsetof(VAULT, fGoadStart));

	g_aeopid[1051].grfeopid = 128;
	g_aeopid[1051].otyp = OTYP_Float;
	g_aeopid[1051].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(PUFFER, tGameMax));

	g_aeopid[1052].grfeopid = 2;
	g_aeopid[1052].otyp = OTYP_Map;
	g_aeopid[1052].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PUFFER, psmaShoot));

	g_aeopid[1053].grfeopid = 2;
	g_aeopid[1053].otyp = OTYP_Map;
	g_aeopid[1053].optdat.ibGet = static_cast<std::intptr_t>(offsetof(PUFFER, psma));

	g_aeopid[1054].grfeopid = 128;
	g_aeopid[1054].otyp = OTYP_Vector;
	g_aeopid[1054].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(MRKV, normal));

	g_aeopid[1055].grfeopid = 128;
	g_aeopid[1055].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(MRKV, sDepth));
	g_aeopid[1055].otyp = OTYP_Float;

	g_aeopid[1056].grfeopid = 128;
	g_aeopid[1056].otyp = OTYP_Lm;
	g_aeopid[1056].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(MRKV, lmAlpha));

	g_aeopid[1057].grfeopid = 146;
	g_aeopid[1057].otyp = OTYP_Int;
	g_aeopid[1057].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKSPAWN, cKillGoal));
	g_aeopid[1057].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKSPAWN, cKillGoal));
	g_aeopid[1057].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKSPAWN, cKillGoal));

	g_aeopid[1058].grfeopid = 146;
	g_aeopid[1058].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKSPAWN, tGameMax));
	g_aeopid[1058].otyp = OTYP_Float;
	g_aeopid[1058].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKSPAWN, tGameMax));
	g_aeopid[1058].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKSPAWN, tGameMax));

	g_aeopid[1059].otyp = OTYP_Oid;
	g_aeopid[1059].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&AddCrfodkSpawnSlot);
	g_aeopid[1059].grfeopid = 256;

	g_aeopid[1060].otyp = OTYP_Float;
	g_aeopid[1060].grfeopid = 4242;
	g_aeopid[1060].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmActiveMax) + offsetof(LM, gMin));
	g_aeopid[1060].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmActiveMax) + offsetof(LM, gMin));
	g_aeopid[1060].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmActiveMax) + offsetof(LM, gMin));
	g_aeopid[1060].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;

	g_aeopid[1061].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmActiveMax) + offsetof(LM, gMax));
	g_aeopid[1061].otyp = OTYP_Float;
	g_aeopid[1061].grfeopid = 4242;
	g_aeopid[1061].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmActiveMax) + offsetof(LM, gMax));
	g_aeopid[1061].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmActiveMax) + offsetof(LM, gMax));
	g_aeopid[1061].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;

	g_aeopid[1062].otyp = OTYP_Clq;
	g_aeopid[1062].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToActiveMax));
	g_aeopid[1062].grfeopid = 4242;
	g_aeopid[1062].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToActiveMax));
	g_aeopid[1062].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToActiveMax));
	g_aeopid[1062].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;

	g_aeopid[1063].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, uRespawnEnable));
	g_aeopid[1063].otyp = OTYP_Float;
	g_aeopid[1063].grfeopid = 4242;
	g_aeopid[1063].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, uRespawnEnable));
	g_aeopid[1063].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, uRespawnEnable));
	g_aeopid[1063].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;

	g_aeopid[1064].otyp = OTYP_Float;
	g_aeopid[1064].grfeopid = 4242;
	g_aeopid[1064].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmSpawnDelayStart) + offsetof(LM, gMin));
	g_aeopid[1064].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmSpawnDelayStart) + offsetof(LM, gMin));
	g_aeopid[1064].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;
	g_aeopid[1064].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmSpawnDelayStart) + offsetof(LM, gMin));

	g_aeopid[1065].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmSpawnDelayEnd) + offsetof(LM, gMin));
	g_aeopid[1065].otyp = OTYP_Float;
	g_aeopid[1065].grfeopid = 4242;
	g_aeopid[1065].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmSpawnDelayEnd) + offsetof(LM, gMin));
	g_aeopid[1065].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;
	g_aeopid[1065].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmSpawnDelayEnd) + offsetof(LM, gMin));

	g_aeopid[1066].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToSpawnDelay));
	g_aeopid[1066].otyp = OTYP_Clq;
	g_aeopid[1066].grfeopid = 4242;
	g_aeopid[1066].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToSpawnDelay));
	g_aeopid[1066].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;
	g_aeopid[1066].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToSpawnDelay));

	g_aeopid[1067].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmRespawnDelayStart) + offsetof(LM, gMin));
	g_aeopid[1067].otyp = OTYP_Float;
	g_aeopid[1067].grfeopid = 4242;
	g_aeopid[1067].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmRespawnDelayStart) + offsetof(LM, gMin));
	g_aeopid[1067].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;
	g_aeopid[1067].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmRespawnDelayStart) + offsetof(LM, gMin));

	g_aeopid[1068].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmRespawnDelayEnd) + offsetof(LM, gMin));
	g_aeopid[1068].otyp = OTYP_Float;
	g_aeopid[1068].grfeopid = 4242;
	g_aeopid[1068].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmRespawnDelayEnd) + offsetof(LM, gMin));
	g_aeopid[1068].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;
	g_aeopid[1068].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmRespawnDelayEnd) + offsetof(LM, gMin));

	g_aeopid[1069].otyp = OTYP_Clq;
	g_aeopid[1069].grfeopid = 4242;
	g_aeopid[1069].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToRespawnDelay));
	g_aeopid[1069].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToRespawnDelay));
	g_aeopid[1069].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToRespawnDelay));
	g_aeopid[1069].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;

	g_aeopid[1070].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmDifficultyValue) + offsetof(LM, gMin));
	g_aeopid[1070].otyp = OTYP_Float;
	g_aeopid[1070].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmDifficultyValue) + offsetof(LM, gMin));
	g_aeopid[1070].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;
	g_aeopid[1070].grfeopid = 4242;
	g_aeopid[1070].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmDifficultyValue) + offsetof(LM, gMin));

	g_aeopid[1071].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmDifficultyValue) + offsetof(LM, gMax));
	g_aeopid[1071].otyp = OTYP_Float;
	g_aeopid[1071].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmDifficultyValue) + offsetof(LM, gMax));
	g_aeopid[1071].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;
	g_aeopid[1071].grfeopid = 4242;
	g_aeopid[1071].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmDifficultyValue) + offsetof(LM, gMax));

	g_aeopid[1072].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToDifficulty));
	g_aeopid[1072].otyp = OTYP_Clq;
	g_aeopid[1072].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToDifficulty));
	g_aeopid[1072].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;
	g_aeopid[1072].grfeopid = 4242;
	g_aeopid[1072].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToDifficulty));

	g_aeopid[1073].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmSecondaryDifficulty) + offsetof(LM, gMin));
	g_aeopid[1073].otyp = OTYP_Float;
	g_aeopid[1073].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmSecondaryDifficulty) + offsetof(LM, gMin));
	g_aeopid[1073].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;
	g_aeopid[1073].grfeopid = 4242;
	g_aeopid[1073].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmSecondaryDifficulty) + offsetof(LM, gMin));

	g_aeopid[1074].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmSecondaryDifficulty) + offsetof(LM, gMax));
	g_aeopid[1074].otyp = OTYP_Float;
	g_aeopid[1074].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmSecondaryDifficulty) + offsetof(LM, gMax));
	g_aeopid[1074].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;
	g_aeopid[1074].grfeopid = 4242;
	g_aeopid[1074].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, lmSecondaryDifficulty) + offsetof(LM, gMax));

	g_aeopid[1075].otyp = OTYP_Clq;
	g_aeopid[1075].grfeopid = 4242;
	g_aeopid[1075].optdat.pfnensure = (PFNENSURE)PcrfodkSpawnerDifficultyData;
	g_aeopid[1075].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToSecondary));
	g_aeopid[1075].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToSecondary));
	g_aeopid[1075].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKTUNE, clqProgressToSecondary));

	g_aeopid[1076].otyp = OTYP_Int;
	g_aeopid[1076].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKSPAWN, iDifficultyData));
	g_aeopid[1076].grfeopid = 146;
	g_aeopid[1076].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKSPAWN, iDifficultyData));
	g_aeopid[1076].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKSPAWN, iDifficultyData));

	g_aeopid[1077].otyp = OTYP_Clq;
	g_aeopid[1077].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKSPAWN, clqSuckToTune));
	g_aeopid[1077].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODKSPAWN, clqSuckToTune));
	g_aeopid[1077].grfeopid = 146;
	g_aeopid[1077].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODKSPAWN, clqSuckToTune));

	g_aeopid[1078].otyp = OTYP_Map;
	g_aeopid[1078].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODKSPAWN, psma));
	g_aeopid[1078].grfeopid = 2;

	g_aeopid[1079].otyp = OTYP_Int;
	g_aeopid[1079].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CRFODB, cpoint));
	g_aeopid[1079].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CRFODB, cpoint));
	g_aeopid[1079].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CRFODB, cpoint));
	g_aeopid[1079].grfeopid = 146;

	g_aeopid[1080].otyp = OTYP_Float;
	g_aeopid[1080].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, sRadiusFrontWheel));
	g_aeopid[1080].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, sRadiusFrontWheel));
	g_aeopid[1080].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SUV, sRadiusFrontWheel));
	g_aeopid[1080].grfeopid = 146;

	g_aeopid[1081].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SUV, sRadiusRearWheel));
	g_aeopid[1081].otyp = OTYP_Float;
	g_aeopid[1081].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, sRadiusRearWheel));
	g_aeopid[1081].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, sRadiusRearWheel));
	g_aeopid[1081].grfeopid = 146;

	g_aeopid[1082].otyp = OTYP_Float;
	g_aeopid[1082].grfeopid = 146;
	g_aeopid[1082].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, svMax));
	g_aeopid[1082].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, svMax));
	g_aeopid[1082].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SUV, svMax));

	g_aeopid[1083].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SUV, dyMax));
	g_aeopid[1083].otyp = OTYP_Float;
	g_aeopid[1083].grfeopid = 146;
	g_aeopid[1083].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, dyMax));
	g_aeopid[1083].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, dyMax));

	g_aeopid[1084].otyp = OTYP_Rope;
	g_aeopid[1084].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, pshapeTrack));
	g_aeopid[1084].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSuvTrack);
	g_aeopid[1084].grfeopid = 34;

	g_aeopid[1085].otyp = OTYP_Rope;
	g_aeopid[1085].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, pshapeLine));
	g_aeopid[1085].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSuvLine);
	g_aeopid[1085].grfeopid = 34;

	g_aeopid[1086].otyp = OTYP_Int;
	g_aeopid[1086].grfeopid = 146;
	g_aeopid[1086].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, cLapMax));
	g_aeopid[1086].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, cLapMax));
	g_aeopid[1086].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SUV, cLapMax));

	g_aeopid[1087].otyp = OTYP_Void;
	g_aeopid[1087].grfeopid = 1;
	g_aeopid[1087].optdat.ibSet = 1;
	g_aeopid[1087].optdat.pvThunkFn = RefThunkSuvADD_CHECK_POINT;
	g_aeopid[1087].optdat.ibGet = 0;

	g_aeopid[1088].otyp = OTYP_Void;
	g_aeopid[1088].grfeopid = 1;
	g_aeopid[1088].optdat.ibSet = 5;
	g_aeopid[1088].optdat.pvThunkFn = RefThunkSuvADD_FEATURE;
	g_aeopid[1088].optdat.ibGet = 0;

	g_aeopid[1089].otyp = OTYP_Pntsv;
	g_aeopid[1089].grfeopid = 18;
	g_aeopid[1089].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, ppathzone));
	g_aeopid[1089].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, ppathzone));

	g_aeopid[1090].otyp = 4183;
	g_aeopid[1090].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, suvs));
	g_aeopid[1090].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetSuvSuvs);
	g_aeopid[1090].grfeopid = 290;
	g_aeopid[1090].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetSuvSuvs);

	g_aeopid[1091].grfeopid = 1;
	g_aeopid[1091].otyp = OTYP_Void;
	g_aeopid[1091].optdat.ibGet = 0;
	g_aeopid[1091].optdat.ibSet = 0;
	g_aeopid[1091].optdat.pvThunkFn = RefThunkSuvRESET;

	g_aeopid[1092].grfeopid = 146;
	g_aeopid[1092].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, suvgk));
	g_aeopid[1092].otyp = OTYP_Jsglk;
	g_aeopid[1092].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, suvgk));
	g_aeopid[1092].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SUV, suvgk));

	g_aeopid[1093].grfeopid = 146;
	g_aeopid[1093].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, cLapMax));
	g_aeopid[1093].otyp = OTYP_Int;
	g_aeopid[1093].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, cLapMax));
	g_aeopid[1093].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SUV, cLapMax));

	g_aeopid[1094].grfeopid = 18;
	g_aeopid[1094].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, pexplDirt));
	g_aeopid[1094].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, pexplDirt));
	g_aeopid[1094].otyp = OTYP_Cm;

	g_aeopid[1095].grfeopid = 18;
	g_aeopid[1095].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, pexplDust));
	g_aeopid[1095].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, pexplDust));
	g_aeopid[1095].otyp = OTYP_Cm;

	g_aeopid[1096].grfeopid = 18;
	g_aeopid[1096].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, pemitterBoost));
	g_aeopid[1096].otyp = OTYP_Pufft;
	g_aeopid[1096].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, pemitterBoost));

	g_aeopid[1097].otyp = OTYP_Float;
	g_aeopid[1097].grfeopid = 146;
	g_aeopid[1097].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, muSxp));
	g_aeopid[1097].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SUV, muSxp));
	g_aeopid[1097].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, muSxp));

	g_aeopid[1098].otyp = OTYP_Int;
	g_aeopid[1098].grfeopid = 146;
	g_aeopid[1098].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, cBoost));
	g_aeopid[1098].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SUV, cBoost));
	g_aeopid[1098].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, cBoost));

	g_aeopid[1099].otyp = OTYP_Clq;
	g_aeopid[1099].grfeopid = 146;
	g_aeopid[1099].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, svrb) + offsetof(SVRB, clqDuToRsvAhead));
	g_aeopid[1099].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SUV, svrb) + offsetof(SVRB, clqDuToRsvAhead));
	g_aeopid[1099].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, svrb) + offsetof(SVRB, clqDuToRsvAhead));

	g_aeopid[1100].otyp = OTYP_Lm;
	g_aeopid[1100].grfeopid = 146;
	g_aeopid[1100].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, svrb) + offsetof(SVRB, clqDuToRsvBehind));
	g_aeopid[1100].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SUV, svrb) + offsetof(SVRB, clqDuToRsvBehind));
	g_aeopid[1100].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, svrb) + offsetof(SVRB, clqDuToRsvBehind));

	g_aeopid[1101].otyp = OTYP_Clq;
	g_aeopid[1101].grfeopid = 146;
	g_aeopid[1101].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, svrb) + offsetof(SVRB, lmRsvRandom));
	g_aeopid[1101].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SUV, svrb) + offsetof(SVRB, lmRsvRandom));
	g_aeopid[1101].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, svrb) + offsetof(SVRB, lmRsvRandom));

	g_aeopid[1102].otyp = OTYP_Lm;
	g_aeopid[1102].grfeopid = 146;
	g_aeopid[1102].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SUV, svrb) + offsetof(SVRB, mpcpsuvuBoost));
	g_aeopid[1102].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SUV, svrb) + offsetof(SVRB, mpcpsuvuBoost));
	g_aeopid[1102].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SUV, svrb) + offsetof(SVRB, mpcpsuvuBoost));


	g_aeopid[1103].grfeopid = 1;
	g_aeopid[1103].otyp = OTYP_Bool;
	g_aeopid[1103].optdat.ibGet = 0;
	g_aeopid[1103].optdat.ibSet = 1;
	g_aeopid[1103].optdat.pvThunkFn = RefThunkVolCHECK_POINT;

	g_aeopid[1104].grfeopid = 1;
	g_aeopid[1104].otyp = OTYP_Bool;
	g_aeopid[1104].optdat.ibGet = 0;
	g_aeopid[1104].optdat.ibSet = 1;
	g_aeopid[1104].optdat.pvThunkFn = RefThunkFlashPOS_WITHIN;

	g_aeopid[1105].grfeopid = 1;
	g_aeopid[1105].otyp = OTYP_Bool;
	g_aeopid[1105].optdat.ibSet = 1;
	g_aeopid[1105].optdat.ibGet = 0;
	g_aeopid[1105].optdat.pvThunkFn = RefThunkSmartguardFAdd_MURRAY_DETECTED_ENEMY;

	g_aeopid[1106].grfeopid = 34;
	g_aeopid[1106].otyp = OTYP_Jackb;
	g_aeopid[1106].optdat.ibGet = static_cast<std::intptr_t>(offsetof(MURRAY, pxfmPursue));
	g_aeopid[1106].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetMurrayPursueXfm);

	g_aeopid[1107].grfeopid = 34;
	g_aeopid[1107].otyp = OTYP_Jackb;
	g_aeopid[1107].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetMurrayPatrolXfm);
	g_aeopid[1107].optdat.ibGet = static_cast<std::intptr_t>(offsetof(MURRAY, pxfmPatrol));

	g_aeopid[1108].grfeopid = 4;
	g_aeopid[1108].otyp = OTYP_Vector;
	g_aeopid[1108].optdat.pvThunkFnUser = (PVTHUNKFNUSER)ChoosePathzoneRandomPoint;

	g_aeopid[1109].grfeopid = 146;
	g_aeopid[1109].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SWP, smpImpact) + offsetof(SMP, svFast));
	g_aeopid[1109].otyp = OTYP_Float;
	g_aeopid[1109].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SWP, smpImpact) + offsetof(SMP, svFast));
	g_aeopid[1109].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SWP, smpImpact) + offsetof(SMP, svFast));

	g_aeopid[1110].grfeopid = 146;
	g_aeopid[1110].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SWP, smpImpact) + offsetof(SMP, svSlow));
	g_aeopid[1110].otyp = OTYP_Float;
	g_aeopid[1110].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SWP, smpImpact) + offsetof(SMP, svSlow));
	g_aeopid[1110].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SWP, smpImpact) + offsetof(SMP, svSlow));

	g_aeopid[1111].grfeopid = 146;
	g_aeopid[1111].otyp = OTYP_Float;
	g_aeopid[1111].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SWP, smpImpact) + offsetof(SMP, dtFast));
	g_aeopid[1111].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SWP, smpImpact) + offsetof(SMP, dtFast));
	g_aeopid[1111].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SWP, smpImpact) + offsetof(SMP, dtFast));

	g_aeopid[1112].grfeopid = 146;
	g_aeopid[1112].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(SWP, sdvMax));
	g_aeopid[1112].optdat.ibGet = static_cast<std::intptr_t>(offsetof(SWP, sdvMax));
	g_aeopid[1112].otyp = OTYP_Float;
	g_aeopid[1112].optdat.ibSet = static_cast<std::intptr_t>(offsetof(SWP, sdvMax));

	g_aeopid[1113].otyp = OTYP_Void;
	g_aeopid[1113].grfeopid = 1;
	g_aeopid[1113].optdat.ibGet = 0;
	g_aeopid[1113].optdat.ibSet = 2;
	g_aeopid[1113].optdat.pvThunkFn = RefThunkSwpSET_SHAPE;

	g_aeopid[1114].optdat.ibGet = static_cast<std::intptr_t>(offsetof(LGN, lgns));
	g_aeopid[1114].otyp = OTYP_Jtpdk;
	g_aeopid[1114].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetLgnLgns);
	g_aeopid[1114].grfeopid = 34;

	g_aeopid[1115].optdat.pvThunkFnUser = RefThunkJsgCLEAR;
	g_aeopid[1115].otyp = OTYP_Void;
	g_aeopid[1115].grfeopid = 1025;
	g_aeopid[1115].optdat.pvThunkFn = RefThunkJsgCLEAR;
	g_aeopid[1115].optdat.ibSet = 0;

	g_aeopid[1116].otyp = OTYP_Void;
	g_aeopid[1116].grfeopid = 1;
	g_aeopid[1116].optdat.ibGet = 0;
	g_aeopid[1116].optdat.ibSet = 1;
	g_aeopid[1116].optdat.pvThunkFn = RefThunkJsgAPPLY;

	g_aeopid[1117].otyp = OTYP_Void;
	g_aeopid[1117].grfeopid = 1;
	g_aeopid[1117].optdat.ibGet = 0;
	g_aeopid[1117].optdat.pvThunkFn = RefThunkJsgRETRACT;
	g_aeopid[1117].optdat.ibSet = 0;

	g_aeopid[1118].otyp = OTYP_Void;
	g_aeopid[1118].grfeopid = 1025;
	g_aeopid[1118].optdat.pvThunkFn = RefThunkJsgCONTEXT;
	g_aeopid[1118].optdat.pvThunkFnUser = RefThunkJsgCONTEXT;
	g_aeopid[1118].optdat.ibSet = 1;

	g_aeopid[1119].optdat.pvThunkFnUser = RefThunkJsgCUT;
	g_aeopid[1119].otyp = OTYP_Void;
	g_aeopid[1119].optdat.pvThunkFn = RefThunkJsgCUT;
	g_aeopid[1119].optdat.ibSet = 1;
	g_aeopid[1119].grfeopid = 1025;

	g_aeopid[1120].optdat.pvThunkFnUser = RefThunkJsgFOCUS;
	g_aeopid[1120].otyp = OTYP_Void;
	g_aeopid[1120].optdat.pvThunkFn = RefThunkJsgFOCUS;
	g_aeopid[1120].optdat.ibSet = 1;
	g_aeopid[1120].grfeopid = 1025;

	g_aeopid[1121].optdat.pvThunkFnUser = RefThunkJsgLABEL;
	g_aeopid[1121].otyp = OTYP_Void;
	g_aeopid[1121].optdat.pvThunkFn = RefThunkJsgLABEL;
	g_aeopid[1121].optdat.ibSet = 1;
	g_aeopid[1121].grfeopid = 1025;

	g_aeopid[1122].optdat.pvThunkFnUser = RefThunkJsgTUNNEL;
	g_aeopid[1122].optdat.ibSet = 1;
	g_aeopid[1122].otyp = OTYP_Void;
	g_aeopid[1122].optdat.pvThunkFn = RefThunkJsgTUNNEL;
	g_aeopid[1122].grfeopid = 1025;

	g_aeopid[1123].optdat.pvThunkFnUser = RefThunkJsgPAUSE;
	g_aeopid[1123].optdat.ibSet = 2;
	g_aeopid[1123].otyp = OTYP_Void;
	g_aeopid[1123].optdat.pvThunkFn = RefThunkJsgPAUSE;
	g_aeopid[1123].grfeopid = 1025;

	g_aeopid[1124].otyp = OTYP_Void;
	g_aeopid[1124].optdat.pvThunkFnUser = RefThunkJsgJUMP;
	g_aeopid[1124].optdat.ibSet = 4;
	g_aeopid[1124].optdat.pvThunkFn = RefThunkJsgJUMP;
	g_aeopid[1124].grfeopid = 1025;

	g_aeopid[1125].otyp = OTYP_Void;
	g_aeopid[1125].grfeopid = 1025;
	g_aeopid[1125].optdat.ibSet = 3;
	g_aeopid[1125].optdat.pvThunkFnUser = RefThunkJsgRUN;
	g_aeopid[1125].optdat.pvThunkFn = RefThunkJsgRUN;

	g_aeopid[1126].optdat.pvThunkFnUser = RefThunkJsgCLIMB;
	g_aeopid[1126].grfeopid = 1025;
	g_aeopid[1126].optdat.ibSet = 3;
	g_aeopid[1126].otyp = OTYP_Void;
	g_aeopid[1126].optdat.pvThunkFn = RefThunkJsgCLIMB;

	g_aeopid[1127].optdat.pvThunkFnUser = RefThunkJsgATTACK;
	g_aeopid[1127].grfeopid = 1025;
	g_aeopid[1127].optdat.ibSet = 3;
	g_aeopid[1127].otyp = OTYP_Void;
	g_aeopid[1127].optdat.pvThunkFn = RefThunkJsgATTACK;

	g_aeopid[1128].optdat.pvThunkFnUser = RefThunkJsgASEG;
	g_aeopid[1128].grfeopid = 1025;
	g_aeopid[1128].optdat.ibSet = 2;
	g_aeopid[1128].otyp = OTYP_Void;
	g_aeopid[1128].optdat.pvThunkFn = RefThunkJsgASEG;

	g_aeopid[1129].optdat.pvThunkFnUser = RefThunkJsgCLOCK;
	g_aeopid[1129].grfeopid = 1025;
	g_aeopid[1129].optdat.ibSet = 1;
	g_aeopid[1129].otyp = OTYP_Void;
	g_aeopid[1129].optdat.pvThunkFn = RefThunkJsgCLOCK;

	g_aeopid[1130].grfeopid = 0x92;
	g_aeopid[1130].otyp = OTYP_Bool;
	g_aeopid[1130].optdat.ibGet = static_cast<std::ptrdiff_t>(offsetof(JSG, fHandsOff));
	g_aeopid[1130].optdat.ibSet = static_cast<std::ptrdiff_t>(offsetof(JSG, fHandsOff));
	g_aeopid[1130].optdat.ibSetUser = static_cast<std::ptrdiff_t>(offsetof(JSG, fHandsOff));

	g_aeopid[1131].optdat.pvThunkFnUser = RefThunkJsgHIDE;
	g_aeopid[1131].grfeopid = 1025;
	g_aeopid[1131].otyp = OTYP_Void;
	g_aeopid[1131].optdat.pvThunkFn = RefThunkJsgHIDE;
	g_aeopid[1131].optdat.ibSet = 0;

	g_aeopid[1132].otyp = OTYP_Void;
	g_aeopid[1132].grfeopid = 0x401;
	g_aeopid[1132].optdat.pvThunkFnUser = RefThunkThndFlashSTART;
	g_aeopid[1132].optdat.pvThunkFn = RefThunkThndFlashSTART;
	g_aeopid[1132].optdat.ibSet = 0;
	g_aeopid[1133].otyp = OTYP_Rgba;
	g_aeopid[1133].grfeopid = 0x92;
	g_aeopid[1133].optdat.ibSetUser = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, rgbaStart));
	g_aeopid[1133].optdat.ibGet = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, rgbaStart));
	g_aeopid[1133].optdat.ibSet = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, rgbaStart));
	g_aeopid[1134].otyp = OTYP_Rgba;
	g_aeopid[1134].grfeopid = 0x92;
	g_aeopid[1134].optdat.ibSetUser = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, rgbaPeak));
	g_aeopid[1134].optdat.ibGet = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, rgbaPeak));
	g_aeopid[1134].optdat.ibSet = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, rgbaPeak));
	g_aeopid[1135].otyp = OTYP_Rgba;
	g_aeopid[1135].grfeopid = 0x92;
	g_aeopid[1135].optdat.ibSetUser = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, rgbaEnd));
	g_aeopid[1135].optdat.ibGet = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, rgbaEnd));
	g_aeopid[1135].optdat.ibSet = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, rgbaEnd));
	g_aeopid[1136].otyp = OTYP_Float;
	g_aeopid[1136].grfeopid = 0x92;
	g_aeopid[1136].optdat.ibSetUser = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, dtFadeIn));
	g_aeopid[1136].optdat.ibGet = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, dtFadeIn));
	g_aeopid[1136].optdat.ibSet = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, dtFadeIn));
	g_aeopid[1137].otyp = OTYP_Float;
	g_aeopid[1137].grfeopid = 0x92;
	g_aeopid[1137].optdat.ibSetUser = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, dtFadeOut));
	g_aeopid[1137].optdat.ibGet = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, dtFadeOut));
	g_aeopid[1137].optdat.ibSet = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, dtFadeOut));
	g_aeopid[1138].otyp = OTYP_Float;
	g_aeopid[1138].grfeopid = 0x92;
	g_aeopid[1138].optdat.ibGet = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, dtHold));
	g_aeopid[1138].optdat.ibSet = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, dtHold));
	g_aeopid[1138].optdat.ibSetUser = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, dtHold));
	g_aeopid[1139].otyp = 0x105c;
	g_aeopid[1139].grfeopid = 0x92;
	g_aeopid[1139].optdat.ibSetUser = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, thndflashk));
	g_aeopid[1139].optdat.ibSet = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, thndflashk));
	g_aeopid[1139].optdat.ibGet = static_cast<std::ptrdiff_t>(offsetof(THNDFLASH, thndflashk));
	g_aeopid[1140].otyp = OTYP_So;
	g_aeopid[1140].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CLKWORK, psoWeakPoint));
	g_aeopid[1140].grfeopid = 18;
	g_aeopid[1140].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CLKWORK, psoWeakPoint));
	g_aeopid[1141].otyp = static_cast<OTYP>(OTYP_Jsglk | OTYP_Oid);
	g_aeopid[1141].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetClkworkState);
	g_aeopid[1141].grfeopid = 34;
	g_aeopid[1141].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CLKWORK, cwk));
	g_aeopid[1142].grfeopid = 32;
	g_aeopid[1142].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetClkworkTracking);
	g_aeopid[1142].otyp = OTYP_Bool;
	g_aeopid[1143].otyp = OTYP_Void;
	g_aeopid[1143].grfeopid = 1;
	g_aeopid[1143].optdat.ibSet = 0;
	g_aeopid[1143].optdat.ibGet = 0;
	g_aeopid[1143].optdat.pvThunkFn = RefThunkClkworkPLAY_DAMAGE_ANIMATION;
	g_aeopid[1144].otyp = OTYP_Void;
	g_aeopid[1144].grfeopid = 1;
	g_aeopid[1144].optdat.ibSet = 1;
	g_aeopid[1144].optdat.ibGet = 0;
	g_aeopid[1144].optdat.pvThunkFn = RefThunkClkworkSET_DAMAGE_ANIMATION;
	g_aeopid[1145].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JETPACK, radBodyX));
	g_aeopid[1145].otyp = OTYP_Float;
	g_aeopid[1145].grfeopid = 146;
	g_aeopid[1145].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JETPACK, radBodyX));
	g_aeopid[1145].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JETPACK, radBodyX));
	g_aeopid[1146].otyp = OTYP_Float;
	g_aeopid[1146].grfeopid = 146;
	g_aeopid[1146].optdat.ibGet = static_cast<std::intptr_t>(offsetof(JETPACK, radBodyY));
	g_aeopid[1146].optdat.ibSet = static_cast<std::intptr_t>(offsetof(JETPACK, radBodyY));
	g_aeopid[1146].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(JETPACK, radBodyY));
	g_aeopid[1147].otyp = OTYP_Void;
	g_aeopid[1147].grfeopid = 1;
	g_aeopid[1147].optdat.ibGet = 0;
	g_aeopid[1147].optdat.ibSet = 0;
	g_aeopid[1147].optdat.pvThunkFn = RefThunkJetpackRESET_TRANSFORM;
	g_aeopid[1148].otyp = OTYP_Int;
	g_aeopid[1148].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CLKWRKDMG, cHitsRemaining));
	g_aeopid[1148].optdat.ibSet = static_cast<std::intptr_t>(offsetof(CLKWRKDMG, cHitsRemaining));
	g_aeopid[1148].grfeopid = 0x92;
	g_aeopid[1148].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CLKWRKDMG, cHitsRemaining));
	g_aeopid[1149].otyp = static_cast<OTYP>(OTYP_Jsglk | OTYP_Cid);
	g_aeopid[1149].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetClkwrkDmgState);
	g_aeopid[1149].optdat.ibGet = static_cast<std::intptr_t>(offsetof(CLKWRKDMG, cwdk));
	g_aeopid[1149].grfeopid = 162;
	g_aeopid[1149].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(CLKWRKDMG, cwdk));

	g_aeopid[1150].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RUBY, framesPerBeat));
	g_aeopid[1150].otyp = OTYP_Int;
	g_aeopid[1150].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetRubyFramesPerBeat);
	g_aeopid[1150].grfeopid = 290;
	g_aeopid[1150].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetRubyFramesPerBeat);

	g_aeopid[1151].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RUBY, beatsPerMeasure));
	g_aeopid[1151].otyp = OTYP_Int;
	g_aeopid[1151].optdat.ibSet = reinterpret_cast<std::intptr_t>(&SetRubyBeatsPerMeasure);
	g_aeopid[1151].grfeopid = 290;
	g_aeopid[1151].optdat.ibSetUser = reinterpret_cast<std::intptr_t>(&SetRubyBeatsPerMeasure);

	g_aeopid[1152].optdat.pvThunkFn = RefThunkRubySET_RYTHM_SEQUENCE;
	g_aeopid[1152].otyp = OTYP_Void;
	g_aeopid[1152].optdat.ibSet = 1;
	g_aeopid[1152].grfeopid = 1;
	g_aeopid[1152].optdat.ibGet = 0;

	g_aeopid[1153].otyp = OTYP_Void;
	g_aeopid[1153].grfeopid = 1;
	g_aeopid[1153].optdat.ibGet = 0;
	g_aeopid[1153].optdat.ibSet = 0;
	g_aeopid[1153].optdat.pvThunkFn = RefThunkRubyCLEAR_RYTHM_SEQUENCE;

	g_aeopid[1154].otyp = OTYP_Map;
	g_aeopid[1154].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RUBY, psmaRuby));
	g_aeopid[1154].grfeopid = 2;

	g_aeopid[1155].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RUBY, pxfmGoal));
	g_aeopid[1155].grfeopid = 18;
	g_aeopid[1155].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RUBY, pxfmGoal));
	g_aeopid[1155].otyp = OTYP_Jackb;

	g_aeopid[1156].grfeopid = 1;
	g_aeopid[1156].optdat.ibSet = 2;
	g_aeopid[1156].otyp = OTYP_Void;
	g_aeopid[1156].optdat.ibGet = 0;
	g_aeopid[1156].optdat.pvThunkFn = RefThunkRubyADD_RYTHM_EVENT;

	g_aeopid[1157].grfeopid = 1;
	g_aeopid[1157].optdat.ibSet = 1;
	g_aeopid[1157].otyp = OTYP_Void;
	g_aeopid[1157].optdat.ibGet = 0;
	g_aeopid[1157].optdat.pvThunkFn = RefThunkRubyREMOVE_RYTHM_EVENT;

	g_aeopid[1158].grfeopid = 1;
	g_aeopid[1158].otyp = OTYP_Void;
	g_aeopid[1158].optdat.ibGet = 0;
	g_aeopid[1158].optdat.ibSet = 0;
	g_aeopid[1158].optdat.pvThunkFn = RefThunkRubyDAMAGE_BOSS;

	g_aeopid[1159].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RYTHM, pruby));
	g_aeopid[1159].grfeopid = 18;
	g_aeopid[1159].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RYTHM, pruby));
	g_aeopid[1159].otyp = OTYP_Jackb;

	g_aeopid[1160].optdat.ibSet = static_cast<std::intptr_t>(offsetof(RYTHM, pjsgFinish));
	g_aeopid[1160].grfeopid = 18;
	g_aeopid[1160].optdat.ibGet = static_cast<std::intptr_t>(offsetof(RYTHM, pjsgFinish));
	g_aeopid[1160].otyp = OTYP_Spire;

	g_aeopid[1161].otyp = OTYP_Void;
	g_aeopid[1161].optdat.pvThunkFnUser = RefThunkRythmSequenceADD_MATCH;
	g_aeopid[1161].grfeopid = 1025;
	g_aeopid[1161].optdat.ibSet = 3;
	g_aeopid[1161].optdat.pvThunkFn = RefThunkRythmSequenceADD_MATCH;

	g_aeopid[1162].grfeopid = 1025;
	g_aeopid[1162].optdat.ibSet = 3;
	g_aeopid[1162].optdat.pvThunkFnUser = RefThunkRythmSequenceADD_SPLICE_EVENT;
	g_aeopid[1162].otyp = OTYP_Void;
	g_aeopid[1162].optdat.pvThunkFn = RefThunkRythmSequenceADD_SPLICE_EVENT;

	g_aeopid[1163].grfeopid = 1025;
	g_aeopid[1163].optdat.ibSet = 2;
	g_aeopid[1163].optdat.pvThunkFnUser = RefThunkRythmSequenceENABLE_INPUT;
	g_aeopid[1163].otyp = OTYP_Void;
	g_aeopid[1163].optdat.pvThunkFn = RefThunkRythmSequenceENABLE_INPUT;

	g_aeopid[1164].grfeopid = 1025;
	g_aeopid[1164].optdat.ibSet = 2;
	g_aeopid[1164].optdat.pvThunkFnUser = RefThunkRythmSequenceDISABLE_INPUT;
	g_aeopid[1164].otyp = OTYP_Void;
	g_aeopid[1164].optdat.pvThunkFn = RefThunkRythmSequenceDISABLE_INPUT;

	g_aeopid[1165].grfeopid = 1025;
	g_aeopid[1165].optdat.ibSet = 2;
	g_aeopid[1165].optdat.pvThunkFnUser = RefThunkRythmSequenceENABLE_EFFECT;
	g_aeopid[1165].otyp = OTYP_Void;
	g_aeopid[1165].optdat.pvThunkFn = RefThunkRythmSequenceENABLE_EFFECT;

	g_aeopid[1166].grfeopid = 1025;
	g_aeopid[1166].optdat.ibSet = 2;
	g_aeopid[1166].optdat.pvThunkFnUser = RefThunkRythmSequenceDISABLE_EFFECT;
	g_aeopid[1166].otyp = OTYP_Void;
	g_aeopid[1166].optdat.pvThunkFn = RefThunkRythmSequenceDISABLE_EFFECT;

	g_aeopid[1167].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BHG, cAbsorbedHits));
	g_aeopid[1167].grfeopid = 0x12;
	g_aeopid[1167].otyp = OTYP_Int;
	g_aeopid[1167].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BHG, cAbsorbedHits));

	g_aeopid[1168].otyp = OTYP_Void;
	g_aeopid[1168].grfeopid = 1;
	g_aeopid[1168].optdat.pvThunkFn = RefThunkBhgSET_HIT_ANIMATIONS;
	g_aeopid[1168].optdat.crefReq = 3;

	g_aeopid[1169].optdat.ibSet = static_cast<std::intptr_t>(offsetof(BHG, fDetect));
	g_aeopid[1169].optdat.ibGet = static_cast<std::intptr_t>(offsetof(BHG, fDetect));
	g_aeopid[1169].otyp = OTYP_Bool;
	g_aeopid[1169].grfeopid = 0x12;

	g_aeopid[1170].otyp = OTYP_Mgvs;
	g_aeopid[1170].optdat.ibGet = static_cast<std::intptr_t>(offsetof(MGV, mgvs));
	g_aeopid[1170].optdat.pfnset = reinterpret_cast<PFNRAW>(SetMgvMgvs);
	g_aeopid[1170].grfeopid = 0x22;

	g_aeopid[1171].otyp = OTYP_Void;
	g_aeopid[1171].optdat.crefReq = 1;
	g_aeopid[1171].grfeopid = 1;
	g_aeopid[1171].optdat.pvThunkFn = RefThunkMgvAPPLY_DEATH_THROW;

	g_aeopid[1172].otyp = OTYP_Int;
	g_aeopid[1172].optdat.ibGet = static_cast<std::intptr_t>(offsetof(MGV, cSecurityCodes));
	g_aeopid[1172].optdat.ibSet = static_cast<std::intptr_t>(offsetof(MGV, cSecurityCodes));
	g_aeopid[1172].grfeopid = 0x92;
	g_aeopid[1172].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(MGV, cSecurityCodes));

	g_aeopid[1173].otyp = OTYP_Int;
	g_aeopid[1173].optdat.ibGet = static_cast<std::intptr_t>(offsetof(MGV, clives));
	g_aeopid[1173].optdat.ibSet = static_cast<std::intptr_t>(offsetof(MGV, clives));
	g_aeopid[1173].grfeopid = 0x92;
	g_aeopid[1173].optdat.ibSetUser = static_cast<std::intptr_t>(offsetof(MGV, clives));

	g_aeopid[1174].otyp = OTYP_Void;
	g_aeopid[1174].optdat.crefReq = 1;
	g_aeopid[1174].grfeopid = 1;
	g_aeopid[1174].optdat.pvThunkFn = RefThunkMgvADD_RESPAWN_POINT;

	g_aeopid[1175].otyp = OTYP_Void;
	g_aeopid[1175].grfeopid = 1;
	g_aeopid[1175].optdat.crefReq = 1;
	g_aeopid[1175].optdat.pvThunkFn = RefThunkMgvRESPAWN_OBJECT;

	g_aeopid[1176].otyp = OTYP_Int;
	g_aeopid[1176].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DECOY, cHitsRemaining));
	g_aeopid[1176].grfeopid = 18;
	g_aeopid[1176].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DECOY, cHitsRemaining));

	g_aeopid[1177].grfeopid = 18;
	g_aeopid[1177].otyp = OTYP_Cm;
	g_aeopid[1177].optdat.ibSet = static_cast<std::intptr_t>(offsetof(DECOY, pexplDestroy));
	g_aeopid[1177].optdat.ibGet = static_cast<std::intptr_t>(offsetof(DECOY, pexplDestroy));

	g_aeopid[1178].otyp = OTYP_Void;
	g_aeopid[1178].grfeopid = 1;
	g_aeopid[1178].optdat.pvThunkFn = RefThunkDecoyDAMAGE;
	g_aeopid[1178].optdat.ibSet = 2;
	g_aeopid[1178].optdat.ibGet = 0;
}

CRef RefThunkBasicDERIVED_FROMQ(BASIC* pbasic, int carg, CRef* aref)
{
	BASIC* basic = reinterpret_cast<BASIC*>(pbasic);

	CID cid = CID_Nil;

	if (carg > 0)
		cid = static_cast<CID>(aref[0].RefCoerceS32().m_n);

	int fBool = FIsBasicDerivedFrom(basic, cid);

	CRef ret;
	ret.SetBool(fBool);
	return ret;
}

CRef RefThunkLoREMOVEFnUser(BASIC* pbasic, int carg, CRef* aref)
{
	LO* lo = reinterpret_cast<LO*>(pbasic);
	DeferLoRemove(lo);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkLoREMOVEFn(BASIC* pbasic, int carg, CRef* aref)
{
	LO* plo = reinterpret_cast<LO*>(pbasic);
	plo->pvtlo->pfnRemoveLo(plo);
	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkLoADD(BASIC* pbasic, int carg, CRef* aref)
{
	LO* plo = reinterpret_cast<LO*>(pbasic);
	plo->pvtlo->pfnAddLo(plo);
	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkLoCLONE(BASIC* pbasic, int carg, CRef* aref)
{
	LO* plo = reinterpret_cast<LO*>(pbasic);

	SW* psw = nullptr;
	ALO* paloParent = nullptr;

	if (carg > 0)
		psw = reinterpret_cast<SW*>(aref[0].m_pbasic);

	if (carg > 1)
		paloParent = reinterpret_cast<ALO*>(aref[1].m_pbasic);

	LO* ploclone = PloCloneLo(plo, psw, paloParent);

	CRef ret;
	ret.SetBasic(reinterpret_cast<BASIC*>(ploclone));
	return ret;
}

CRef RefThunkLoSETPARENT(BASIC* pbasic, int carg, CRef* aref)
{
	LO* plo = reinterpret_cast<LO*>(pbasic);

	ALO* paloParent = nullptr;

	if (carg > 0)
		paloParent = reinterpret_cast<ALO*>(aref[0].m_pbasic);

	plo->pvtlo->pfnSetLoParent(plo, paloParent);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkLoANCESTORQ(BASIC* pbasic, int carg, CRef* aref)
{
	LO* plo = reinterpret_cast<LO*>(pbasic);

	ALO* paloAncestor = nullptr;

	if (carg > 0)
		paloAncestor = reinterpret_cast <ALO*>(aref[0].m_pbasic);

	int fBool = FFindLoParent(plo, paloAncestor);

	CRef ret;
	ret.SetBool(fBool);
	return ret;
}

CRef RefThunkLoMATCHES_NAMEQ(BASIC* pbasic, int carg, CRef* aref)
{
	LO* plo = reinterpret_cast<LO*>(pbasic);

	OID oid = OID_Nil;

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	int fBool = FMatchesLoName(plo, oid);

	CRef ret;
	ret.SetBool(fBool);
	return ret;
}

CRef RefThunkLoCUSTOM_BOOL_PROPERTY(BASIC* pbasic, int carg, CRef* aref)
{
	LO *plo = reinterpret_cast<LO*>(pbasic);

	OPTID name = (OPTID)0;
	int value = 0;

	if (carg > 0)
		name = static_cast <OPTID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		value = aref[1].m_tagk == TAGK_Bool
		? aref[1].m_bool
		: aref[1].RefCoerceS32().m_n;

	EnsureLoSidebagBool(plo, name, value);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkLoCUSTOM_INT_PROPERTY(BASIC* pbasic, int carg, CRef* aref)
{
	LO* plo = reinterpret_cast<LO*>(pbasic);

	OPTID name = (OPTID)0;
	s32 value = 0;

	if (carg > 0)
		name = static_cast<OPTID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		value = aref[1].RefCoerceS32().m_n;

	EnsureLoSidebagInt(plo, name, value);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkLoCUSTOM_FLOAT_PROPERTY(BASIC* pbasic, int carg, CRef* aref)
{
	LO* plo = reinterpret_cast<LO*>(pbasic);

	OPTID name = (OPTID)0;
	f32 value = 0.0f;

	if (carg > 0)
		name = static_cast<OPTID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		value = aref[1].RefCoerceF32().m_g;

	EnsureLoSidebagFloat(plo, name, value);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkLoCUSTOM_CLQ_PROPERTY(BASIC* pbasic, int carg, CRef* aref)
{
	LO* plo = reinterpret_cast<LO*>(pbasic);

	OPTID name = (OPTID)0;
	CLQ value{};

	if (carg > 0)
		name = static_cast<OPTID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1 && aref[1].m_tagk == TAGK_Clq)
		value = aref[1].m_clq;

	EnsureLoSidebagClq(plo, name, &value);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkLoCUSTOM_LM_PROPERTY(BASIC* pbasic, int carg, CRef* aref)
{
	LO* plo = reinterpret_cast<LO*>(pbasic);

	OPTID name = (OPTID)0;
	LM value{};

	if (carg > 0)
		name = static_cast<OPTID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1 && aref[1].m_tagk == TAGK_Lm)
		value = aref[1].m_lm;

	EnsureLoSidebagLm(plo, name, &value);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkLoCUSTOM_OID_PROPERTY(BASIC* pbasic, int carg, CRef* aref)
{
	LO* plo = reinterpret_cast<LO*>(pbasic);

	OPTID name = (OPTID)0;
	OID value = (OID)0;

	if (carg > 0)
		name = static_cast<OPTID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		value = static_cast<OID>(aref[1].RefCoerceS32().m_n);

	EnsureLoSidebagOid(plo, name, value);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkLoCUSTOM_VECTOR_PROPERTY(BASIC* pbasic, int carg, CRef* aref)
{
	LO* plo = reinterpret_cast<LO*>(pbasic);

	OPTID name = (OPTID)0;
	glm::vec3 value{};

	if (carg > 0)
		name = static_cast<OPTID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1 && aref[1].m_tagk == TAGK_Vector)
		value = aref[1].m_vector;

	EnsureLoSidebagVector(plo, name, &value);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAloRIP_REF(BASIC* pbasic, int cref, CRef* aref)
{
	ALO* palo = reinterpret_cast<ALO*>(pbasic);

	

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;

	return ret;
}

CRef RefThunkAloINVALIDATE_LIGHTING(BASIC* pbasic, int carg, CRef* aref)
{
	InvalidateAloLighting(reinterpret_cast<ALO*>(pbasic));

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAloROTATION_MATCHES_VELOCITY(BASIC* pbasic, int carg, CRef* aref)
{
	ALO* palo = reinterpret_cast<ALO*>(pbasic);

	float bank = 0.0f;
	float predictTime = 0.0f;
	ACK ack = ACK_None;

	if (carg > 0) {
		bank = aref[0].RefCoerceF32().m_g;
	}

	if (carg > 1) {
		predictTime = aref[1].RefCoerceF32().m_g;
	}

	if (carg > 2) {
		ack = static_cast<ACK>(aref[2].RefCoerceS32().m_n);
	}

	SetAloRotationMatchesVelocity(palo, bank, predictTime, ack);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAloSCROLLING_MASTER_SPEEDS(BASIC* pbasic, int carg, CRef* aref)
{
	ALO* palo = reinterpret_cast<ALO*>(pbasic);

	float svu = 0.0f;
	float svv = 0.0f;

	if (carg > 0) {
		svu = aref[0].RefCoerceF32().m_g;
	}

	if (carg > 1) {
		svv = aref[1].RefCoerceF32().m_g;
	}

	SetAloScrollingMasterSpeeds(palo, svu, svv);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAloEYES_CLOSED(BASIC* pbasic, int carg, CRef* aref)
{
	ALO* palo = reinterpret_cast<ALO*>(pbasic);
	float uClosed = carg > 0 ? aref[0].RefCoerceF32().m_g : 0.0f;

	SetAloEyesClosed(palo, uClosed);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAloRESUME_ACTLA(BASIC* pbasic, int carg, CRef* aref)
{
	ResumeAloActla(reinterpret_cast<ALO*>(pbasic));

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAloPAUSE_ACTLA(BASIC* pbasic, int carg, CRef* aref)
{
	PauseAloActla(reinterpret_cast<ALO*>(pbasic));

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAloSTART_SOUND(BASIC* pbasic, int carg, CRef* aref)
{
	ALO* palo = reinterpret_cast<ALO*>(pbasic);

	SFXID sfxid = SFXID_Nil;
	float sStart = 0.0f;
	float sFull = 0.0f;
	float uVolume = 1.0f;
	LM lmRepeat{};

	if (carg > 0) {
		sfxid = static_cast<SFXID>(aref[0].RefCoerceS32().m_n);
	}

	if (carg > 1) {
		sStart = aref[1].RefCoerceF32().m_g;
	}

	if (carg > 2) {
		sFull = aref[2].RefCoerceF32().m_g;
	}

	if (carg > 3) {
		uVolume = aref[3].RefCoerceF32().m_g;
	}

	if (carg > 4 && aref[4].m_tagk == TAGK_Lm) {
		lmRepeat = aref[4].m_lm;
	}

	StartAloSound(palo, sfxid, sStart, sFull, uVolume, &lmRepeat);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAloSTOP_SOUND(BASIC* pbasic, int carg, CRef* aref)
{
	ALO* palo = reinterpret_cast<ALO*>(pbasic);

	StopAloSound(palo);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAloFADE_IN(BASIC* pbasic, int carg, CRef* aref)
{
	ALO* palo = reinterpret_cast<ALO*>(pbasic);

	float dtFade = 0.0f;

	if (carg > 0) {
		dtFade = aref[0].RefCoerceF32().m_g;
	}

	FadeAloIn(palo, dtFade);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAloFADE_OUT(BASIC* pbasic, int carg, CRef* aref)
{
	ALO* palo = reinterpret_cast<ALO*>(pbasic);

	float dtFade = 0.0f;

	if (carg > 0) {
		dtFade = aref[0].RefCoerceF32().m_g;
	}

	FadeAloOut(palo, dtFade);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAloCURRENT_SMA(BASIC* pbasic, int carg, CRef* aref)
{
	ALO* palo = reinterpret_cast<ALO*>(pbasic);

	OID oidSm = OID_Nil;

	if (carg > 0) {
		oidSm = static_cast<OID>(aref[0].RefCoerceS32().m_n);
	}

	SMA* psma = PsmaFindAlo(palo, oidSm);

	CRef ret;
	ret.SetBasic(reinterpret_cast<BASIC*>(psma));
	return ret;
}

CRef RefThunkAloCURRENT_ASEGA(BASIC* pbasic, int carg, CRef* aref)
{
	ALO* palo = reinterpret_cast<ALO*>(pbasic);

	OID oidAseg = OID_Nil;

	if (carg > 0) {
		oidAseg = static_cast<OID>(aref[0].RefCoerceS32().m_n);
	}

	ASEGA* pasega = PasegaFindAlo(palo, oidAseg);

	CRef ret;
	ret.SetBasic(reinterpret_cast<BASIC*>(pasega));
	return ret;
}

CRef RefThunkAloNEAREST_ASEGA(BASIC* pbasic, int carg, CRef* aref)
{
	ALO* palo = reinterpret_cast<ALO*>(pbasic);

	ASEGA* pasega = PasegaFindAloNearest(palo);

	CRef ret;
	ret.SetBasic(reinterpret_cast<BASIC*>(pasega));
	return ret;
}

CRef RefThunkAloMATCH_OTHER_OBJECT(BASIC* pbasic, int carg, CRef* aref)
{
	ALO* palo = reinterpret_cast<ALO*>(pbasic);
	ALO* other = nullptr;

	if (carg > 0) {
		other = reinterpret_cast<ALO*>(aref[0].m_pbasic);
	}

	palo->pvtalo->pfnMatchAloOtherObject(palo, other);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAloSET_TRANSFORM_BASIS(BASIC* pbasic, int carg, CRef* aref)
{
	ALO* palo = reinterpret_cast<ALO*>(pbasic);
	ALO* paloPosBasis = carg > 0 ? reinterpret_cast<ALO*>(aref[0].m_pbasic) : nullptr;
	ALO* paloRotBasis = carg > 1 ? reinterpret_cast<ALO*>(aref[1].m_pbasic) : nullptr;

	SetAloTransformBasis(palo, paloPosBasis, paloRotBasis);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSoEDGE_GRAB(BASIC* pbasic, int carg, CRef* aref)
{
	SO* pso = reinterpret_cast<SO*>(pbasic);

	int edgeGrab = 0;

	if (carg > 0)
		edgeGrab = aref[0].RefCoerceS32().m_n;

	SetSoEdgeGrab(pso, (EGK)edgeGrab);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkPoMAKE_ACTIVE(BASIC* pbasic, int carg, CRef* aref)
{
	PO* ppo = reinterpret_cast<PO*>(pbasic);

	MakePoActive(ppo);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkPoINVULNERABLEQ(BASIC* pbasic, int carg, CRef* aref)
{
	PO* ppo = reinterpret_cast<PO*>(pbasic);

	ZPK zap = (ZPK)-1;

	if (carg > 0)
		zap = static_cast<ZPK>(aref[0].RefCoerceS32().m_n);

	int fBool = ppo->pvtpo->pfnFInvulnerablePo(ppo, zap);

	CRef ret;
	ret.SetBool(fBool);
	return ret;
}

CRef RefThunkJtTHROW(BASIC* pbasic, int carg, CRef* aref)
{
	JT* pjt = reinterpret_cast<JT*>(pbasic);

	glm::vec3 posTarget{};
	float angVert = 0.0f;
	float uSteer = 0.0f;

	if (carg > 0 && aref[0].m_tagk == TAGK_Vector)
		posTarget = aref[0].m_vector;

	if (carg > 1)
		angVert = aref[1].RefCoerceF32().m_g;

	if (carg > 2)
		uSteer = aref[2].RefCoerceF32().m_g;

	ThrowJt(pjt, &posTarget, angVert, uSteer);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJtUNHOOK(BASIC* pbasic, int carg, CRef* aref)
{
	JT* pjt = reinterpret_cast<JT*>(pbasic);

	UnhookJt(pjt);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJtPLACE_ON_PIPE(BASIC* pbasic, int carg, CRef* aref)
{
	JT* pjt = reinterpret_cast<JT*>(pbasic);

	PIPE* ppipe = nullptr;

	if (carg > 0)
		ppipe = reinterpret_cast<PIPE*>(aref[0].m_pbasic);

	PlaceJtOnPipe(pjt, ppipe);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJtSET_PUPPET(BASIC* pbasic, int carg, CRef* aref)
{
	JT* pjt = reinterpret_cast<JT*>(pbasic);
	const int fPuppet = carg > 0 ? aref[0].RefCoerceS32().m_n : 0;

	// The retail thunk decodes this argument as OTYP_Bool, then passes its
	// 0/1 value through SetJtPuppet's pointer-typed parameter. SetJtPuppet only
	// distinguishes null from non-null.
	SetJtPuppet(pjt, reinterpret_cast<EA*>(static_cast<intptr_t>(fPuppet != 0)));

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJtSET_STUN(BASIC* pbasic, int carg, CRef* aref)
{
	JT* pjt = reinterpret_cast<JT*>(pbasic);

	float dtStun = 0.0f;

	if (carg > 0)
		dtStun = aref[0].RefCoerceF32().m_g;

	SetJtStun(pjt, dtStun);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJtDEPLOY_MINE(BASIC* pbasic, int carg, CRef* aref)
{
	JT* pjt = reinterpret_cast<JT*>(pbasic);

	DeployJtMine(pjt);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJtTRIGGER_LOCKG(BASIC* pbasic, int carg, CRef* aref)
{
	JT* pjt = reinterpret_cast<JT*>(pbasic);

	LOCKG* plockg = nullptr;

	if (carg > 0)
		plockg = reinterpret_cast<LOCKG*>(aref[0].m_pbasic);

	TriggerJtLockg(pjt, plockg);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJtTRIGGER_DISPLACE(BASIC* pbasic, int carg, CRef* aref)
{
	JT* pjt = reinterpret_cast<JT*>(pbasic);

	SO* pso = nullptr;

	if (carg > 0)
		pso = reinterpret_cast<SO*>(aref[0].m_pbasic);

	TriggerJtDisplace(pjt, pso);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJtTRIGGER_DISPLACE_SECONDARY(BASIC* pbasic, int carg, CRef* aref)
{
	JT* pjt = reinterpret_cast<JT*>(pbasic);

	SO* pso = nullptr;

	if (carg > 0)
		pso = reinterpret_cast<SO*>(aref[0].m_pbasic);

	TriggerJtDisplaceSecondary(pjt, pso);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkStepguardUSE_ANIMATION(BASIC* pbasic, int carg, CRef* aref)
{
	STEPGUARD* pstepguard = reinterpret_cast<STEPGUARD*>(pbasic);

	SGS state = (SGS)-1;
	OID animation = OID_Nil;

	if (carg > 0)
		state = static_cast<SGS>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		animation = static_cast<OID>(aref[1].RefCoerceS32().m_n);

	UseStepguardAnimation(pstepguard, state, animation);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkStepguardUSE_ANIMATION_IMMEDIATE(BASIC* pbasic, int carg, CRef* aref)
{
	STEPGUARD* pstepguard = reinterpret_cast<STEPGUARD*>(pbasic);

	SGS state = (SGS)-1;
	OID animation = OID_Nil;

	if (carg > 0)
		state = static_cast<SGS>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		animation = static_cast<OID>(aref[1].RefCoerceS32().m_n);

	UseStepguardAnimationImmediate(pstepguard, state, animation);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkStepguardGET_ANIMATION_IMMEDIATE(BASIC* pbasic, int carg, CRef* aref)
{
	CRef ret;

	STEPGUARD* pstepguard = reinterpret_cast<STEPGUARD*>(pbasic);

	if (!pstepguard || carg < 1 || !aref)
	{
		ret.SetTag(TAGK_None);
		return ret;
	}

	SGS sgs = static_cast<SGS>(aref[0].RefCoerceS32().m_n);
	ASEG* paseg = GetStepguardSgsAseg(pstepguard, sgs);

	if (!paseg)
	{
		ret.SetTag(TAGK_None);
		return ret;
	}

	ret.SetBasic(reinterpret_cast<BASIC*>(paseg));
	return ret;
}

CRef RefThunkStepguardMATCH_ANIMATION_PHASE(BASIC* pbasic, int carg, CRef* aref)
{
	STEPGUARD* pstepguard = reinterpret_cast<STEPGUARD*>(pbasic);

	OID oid0 = OID_Nil;
	OID oid1 = OID_Nil;
	OID oid2 = OID_Nil;
	OID oid3 = OID_Nil;

	if (carg > 0)
		oid0 = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		oid1 = static_cast<OID>(aref[1].RefCoerceS32().m_n);

	if (carg > 2)
		oid2 = static_cast<OID>(aref[2].RefCoerceS32().m_n);

	if (carg > 3)
		oid3 = static_cast<OID>(aref[3].RefCoerceS32().m_n);

	MatchStepguardAnimationPhase(pstepguard, oid0, oid1, oid2, oid3);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkStepguardADD_ATTACK_EFFECT(BASIC* pbasic, int carg, CRef* aref)
{
	STEPGUARD* pstepguard = reinterpret_cast<STEPGUARD*>(pbasic);

	OID object = OID_Nil;
	ZPK damageType = ZPK_Nil;

	if (carg > 0)
		object = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		damageType = static_cast<ZPK>(aref[1].RefCoerceS32().m_n);

	AddStepguardEffect(pstepguard, object, damageType);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkStepguardUSE_DEATH_ANIMATION(BASIC* pbasic, int carg, CRef* aref)
{
	STEPGUARD* pstepguard = reinterpret_cast<STEPGUARD*>(pbasic);

	FTAK attack = (FTAK)-1;
	OID animation = OID_Nil;

	if (carg > 0)
		attack = static_cast<FTAK>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		animation = static_cast<OID>(aref[1].RefCoerceS32().m_n);

	UseStepguardDeathAnimation(pstepguard, attack, animation);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkStepguardSET_STATE_EXTERNAL(BASIC* pbasic, int carg, CRef* aref)
{
	STEPGUARD* pstepguard = reinterpret_cast<STEPGUARD*>(pbasic);

	SGS sgs = (SGS)-1;
	ASEG* paseg = nullptr;

	if (carg > 0)
		sgs = static_cast<SGS>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		paseg = reinterpret_cast<ASEG*>(aref[1].m_pbasic);

	SetStepguardSgsExternal(pstepguard, sgs, paseg);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkStepguardUSE_PHYS(BASIC* pbasic, int carg, CRef* aref)
{
	STEPGUARD* pstepguard = reinterpret_cast<STEPGUARD*>(pbasic);

	SGS state = (SGS)-1;
	OID phys = OID_Nil;

	if (carg > 0)
		state = static_cast<SGS>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		phys = static_cast<OID>(aref[1].RefCoerceS32().m_n);

	UseStepguardPhys(pstepguard, state, phys);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkStepguardJUMP(BASIC* pbasic, int carg, CRef* aref)
{
	STEPGUARD* pstepguard = reinterpret_cast<STEPGUARD*>(pbasic);

	pstepguard->pvtstepguard->pfnDoStepguardFreefallJump(pstepguard);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkStepguardSET_PATROL_ANIMATION(BASIC* pbasic, int carg, CRef* aref)
{
	STEPGUARD* pstepguard = reinterpret_cast<STEPGUARD*>(pbasic);

	ASEG* pasegPatrol = nullptr;

	if (carg > 0)
		pasegPatrol = reinterpret_cast<ASEG*>(aref[0].m_pbasic);

	SetStepguardPatrolAnimation(pstepguard, pasegPatrol);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSmartguardUSE_FLASHLIGHT_TARGET(BASIC* pbasic, int carg, CRef* aref)
{
	SMARTGUARD* psmartguard = reinterpret_cast<SMARTGUARD*>(pbasic);

	SGS state = SGS_Nil;
	OID target = OID_Nil;

	if (carg > 0)
		state = static_cast<SGS>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		target = static_cast<OID>(aref[1].RefCoerceS32().m_n);

	UseSmartguardFlashlightTarget(psmartguard, state, target);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkMbgUPDATE_AI(BASIC* pbasic, int carg, CRef* aref)
{
	MBG* pmbg = (MBG*)pbasic;
	UpdateMbgAi(pmbg);

	CRef ref;
	ref.SetTag(TAGK_Void);
    return ref;
}

CRef RefThunkBhgSET_HIT_ANIMATIONS(BASIC* pbasic, int carg, CRef* aref)
{
	BHG* pbhg = reinterpret_cast<BHG*>(pbasic);
	ASEG* pasegHit = carg > 0 ? reinterpret_cast<ASEG*>(aref[0].m_pbasic) : nullptr;
	ASEG* pasegSweepCounterHit = carg > 1 ? reinterpret_cast<ASEG*>(aref[1].m_pbasic) : nullptr;
	ASEG* pasegSmashHit = carg > 2 ? reinterpret_cast<ASEG*>(aref[2].m_pbasic) : nullptr;

	SetBhgHitAnimations(pbhg, pasegHit, pasegSweepCounterHit, pasegSmashHit);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSmartguardFAdd_MURRAY_DETECTED_ENEMY(BASIC* pbasic, int carg, CRef* aref)
{
	MURRAY* pmurray = reinterpret_cast<MURRAY*>(pbasic);

	STEPGUARD* pstepguard = nullptr;

	if (carg > 0) {
		pstepguard = reinterpret_cast<STEPGUARD*>(aref[0].m_pbasic);
	}

	bool fAdded = FAddMurrayDetectedEnemy(pmurray, pstepguard);

	CRef ret;
	ret.SetBool(fAdded);
	return ret;
}

CRef RefThunkVaultADD_GOAD_DIALOG(BASIC* pbasic, int carg, CRef* aref)
{
	VAULT* pvault = reinterpret_cast<VAULT*>(pbasic);

	int goadk = 0;
	OID oidDialog = OID_Nil;

	if (carg > 0)
		goadk = aref[0].RefCoerceS32().m_n;

	if (carg > 1)
		oidDialog = static_cast<OID>(aref[1].RefCoerceS32().m_n);

	AddVaultGoadDialog(pvault, goadk, oidDialog);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSuvADD_CHECK_POINT(BASIC* pbasic, int carg, CRef* aref)
{
	SUV* psuv = reinterpret_cast<SUV*>(pbasic);

	VOL* pvol = nullptr;

	if (carg > 0)
		pvol = reinterpret_cast<VOL*>(aref[0].m_pbasic);

	AddSuvCheckPoint(psuv, pvol);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSuvADD_FEATURE(BASIC* pbasic, int carg, CRef* aref)
{
	SUV* psuv = reinterpret_cast<SUV*>(pbasic);

	VOL* pvol = nullptr;
	float mu = 0.0f;
	float maxSpeed = 0.0f;
	EXPL* pexplDirt = nullptr;
	EXPL* pexplDust = nullptr;

	if (carg > 0)
		pvol = reinterpret_cast<VOL*>(aref[0].m_pbasic);

	if (carg > 1)
		mu = aref[1].RefCoerceF32().m_g;

	if (carg > 2)
		maxSpeed = aref[2].RefCoerceF32().m_g;

	if (carg > 3)
		pexplDirt = reinterpret_cast<EXPL*>(aref[3].m_pbasic);

	if (carg > 4)
		pexplDust = reinterpret_cast<EXPL*>(aref[4].m_pbasic);

	AddSuvFeature(psuv, pvol, mu, maxSpeed, pexplDirt, pexplDust);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSuvRESET(BASIC* pbasic, int carg, CRef* aref)
{
	SUV* psuv = reinterpret_cast<SUV*>(pbasic);

	ResetSuv(psuv);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkThndFlashSTART(BASIC* pbasic, int carg, CRef* aref)
{
	THNDFLASH* pthndflash = reinterpret_cast<THNDFLASH*>(pbasic);
	StartThndFlash(pthndflash);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJetpackRESET_TRANSFORM(BASIC* pbasic, int carg, CRef* aref)
{
	(void)carg;
	(void)aref;

	JETPACK* pjetpack = reinterpret_cast<JETPACK*>(pbasic);
	ResetJetpackTransform(pjetpack);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkClkworkPLAY_DAMAGE_ANIMATION(BASIC* pbasic, int carg, CRef* aref)
{
	(void)carg;
	(void)aref;

	CLKWORK* pclkwork = reinterpret_cast<CLKWORK*>(pbasic);
	PlayClkworkDamageAnimation(pclkwork);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkClkworkSET_DAMAGE_ANIMATION(BASIC* pbasic, int carg, CRef* aref)
{
	CLKWORK* pclkwork = reinterpret_cast<CLKWORK*>(pbasic);
	OID oidDamageAnimation = OID_Nil;

	if (carg > 0)
		oidDamageAnimation = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	SetClkworkDamageAnimation(pclkwork, oidDamageAnimation);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkWaterRESET_THROW_COUNT(BASIC* pbasic, int carg, CRef* aref)
{
	WATER* pwater = reinterpret_cast<WATER*>(pbasic);

	ResetWaterThrowCount(pwater);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkBrkBREAK(BASIC* pbasic, int carg, CRef* aref)
{
	BRK* pbrk = reinterpret_cast<BRK*>(pbasic);
	pbrk->pvtbrk->pfnBreakBrk(pbrk);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkDartgunSTART_TARGET_AREA_CHANGE(BASIC* pbasic, int carg, CRef* aref)
{
	DARTGUN* pdartgun = reinterpret_cast<DARTGUN*>(pbasic);

	StartDartgunTargetAreaChange(pdartgun);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkDartgunADD_TARGET_AREA_TARGET(BASIC* pbasic, int carg, CRef* aref)
{
	DARTGUN* pdartgun = reinterpret_cast<DARTGUN*>(pbasic);

	ALO* paloTarget = nullptr;

	if (carg > 0)
		paloTarget = reinterpret_cast<ALO*>(aref[0].m_pbasic);

	AddDartgunTargetAreaTarget(pdartgun, paloTarget);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwpSET_SHAPE(BASIC* pbasic, int carg, CRef* aref)
{
	SWP* pswp = reinterpret_cast<SWP*>(pbasic);

	SHAPE* pshape = nullptr;
	float tMax = 0.0f;

	if (carg > 0)
		pshape = reinterpret_cast<SHAPE*>(aref[0].m_pbasic);

	if (carg > 1)
		tMax = aref[1].RefCoerceF32().m_g;

	SetSwpShape(pswp, pshape, tMax);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkButtonSET_SM_GOAL(BASIC* pbasic, int carg, CRef* aref)
{
	BUTTON* pbutton = reinterpret_cast<BUTTON*>(pbasic);

	int trigger = 0;
	OID object = OID_Nil;
	OID stateMachine = OID_Nil;
	OID goal = OID_Nil;

	if (carg > 0)
		trigger = aref[0].RefCoerceS32().m_n;

	if (carg > 1)
		object = static_cast<OID>(aref[1].RefCoerceS32().m_n);

	if (carg > 2)
		stateMachine = static_cast<OID>(aref[2].RefCoerceS32().m_n);

	if (carg > 3)
		goal = static_cast<OID>(aref[3].RefCoerceS32().m_n);

	SetButtonRsmg(pbutton, trigger, object, stateMachine, goal);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkVolbtnSET_SM_GOAL(BASIC* pbasic, int carg, CRef* aref)
{
	VOLBTN* pvolbtn = reinterpret_cast<VOLBTN*>(pbasic);

	int trigger = 0;
	OID object = OID_Nil;
	OID stateMachine = OID_Nil;
	OID goal = OID_Nil;

	if (carg > 0)
		trigger = aref[0].RefCoerceS32().m_n;

	if (carg > 1)
		object = static_cast<OID>(aref[1].RefCoerceS32().m_n);

	if (carg > 2)
		stateMachine = static_cast<OID>(aref[2].RefCoerceS32().m_n);

	if (carg > 3)
		goal = static_cast<OID>(aref[3].RefCoerceS32().m_n);

	SetVolbtnRsmg(pvolbtn, trigger, object, stateMachine, goal);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkClueBREAK(BASIC* pbasic, int carg, CRef* aref)
{
	CLUE* pclue = reinterpret_cast<CLUE*>(pbasic);

	BreakClue(pclue);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAlarmTRIGGER(BASIC* pbasic, int carg, CRef* aref)
{
	ALARM* palarm = reinterpret_cast<ALARM*>(pbasic);

	ALTK altk = (ALTK)-1;

	if (carg > 0)
		altk = static_cast<ALTK>(aref[0].RefCoerceS32().m_n);

	TriggerAlarm(palarm, altk);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAlarmSET_SM_GOAL(BASIC* pbasic, int carg, CRef* aref)
{
	ALARM* palarm = reinterpret_cast<ALARM*>(pbasic);

	int trigger = 0;
	OID object = OID_Nil;
	OID state_machine = OID_Nil;
	OID goal = OID_Nil;

	if (carg > 0)
		trigger = aref[0].RefCoerceS32().m_n;

	if (carg > 1)
		object = static_cast<OID>(aref[1].RefCoerceS32().m_n);

	if (carg > 2)
		state_machine = static_cast<OID>(aref[2].RefCoerceS32().m_n);

	if (carg > 3)
		goal = static_cast<OID>(aref[3].RefCoerceS32().m_n);

	SetAlarmRsmg(palarm, trigger, object, state_machine, goal);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSensorENABLE(BASIC* pbasic, int carg, CRef* aref)
{
	SENSOR* psensor = reinterpret_cast<SENSOR*>(pbasic);

	SENSM sensm = (SENSM)-1;

	if (carg > 0)
		sensm = static_cast<SENSM>(aref[0].RefCoerceS32().m_n);

	if (psensor->pvtsensor->pfnEnableSensor != nullptr)
		psensor->pvtsensor->pfnEnableSensor(psensor, sensm);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSensorDISABLE(BASIC* pbasic, int carg, CRef* aref)
{
	SENSOR* psensor = reinterpret_cast<SENSOR*>(pbasic);

	if (psensor->pvtsensor->pfnDisableSensor != nullptr)
		psensor->pvtsensor->pfnDisableSensor(psensor);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkLasenEXTEND(BASIC* pbasic, int carg, CRef* aref)
{
	LASEN* plasen = reinterpret_cast<LASEN*>(pbasic);

	float dtExtend = 0.0f;

	if (carg > 0)
		dtExtend = aref[0].RefCoerceF32().m_g;

	ExtendLasen(plasen, dtExtend);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkLasenRETRACT(BASIC* pbasic, int carg, CRef* aref)
{
	LASEN* plasen = reinterpret_cast<LASEN*>(pbasic);

	float dtRetract = 0.0f;

	if (carg > 0)
		dtRetract = aref[0].RefCoerceF32().m_g;

	RetractLasen(plasen, dtRetract);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkTzpRESET_TZP_THROW_COUNT(BASIC* pbasic, int carg, CRef* aref)
{
	TZP* ptzp = reinterpret_cast<TZP*>(pbasic);

	ResetTzpThrowCount(ptzp);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkBombPRIME(BASIC* pbasic, int carg, CRef* aref)
{
	BOMB* pbomb = reinterpret_cast<BOMB*>(pbasic);

	float dt = 0.0f;

	if (carg > 0)
		dt = aref[0].RefCoerceF32().m_g;

	PrimeBomb(pbomb, dt);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkBombDETONATE(BASIC* pbasic, int carg, CRef* aref)
{
	BOMB* pbomb = reinterpret_cast<BOMB*>(pbasic);

	DetonateBomb(pbomb);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkMissileADD_IGNORE_OBJECT(BASIC* pbasic, int carg, CRef* aref)
{
	MISSILE* pmissile = reinterpret_cast<MISSILE*>(pbasic);

	OID oid = OID_Nil;

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	AddMissileIgnoreObject(pmissile, oid);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkMissileADD_IGNORE_CLASS(BASIC* pbasic, int carg, CRef* aref)
{
	MISSILE* pmissile = reinterpret_cast<MISSILE*>(pbasic);

	CID cid = CID_Nil;

	if (carg > 0)
		cid = static_cast<CID>(aref[0].RefCoerceS32().m_n);

	AddMissileIgnoreClass(pmissile, cid);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJloACTIVATE(BASIC* pbasic, int carg, CRef* aref)
{
	JLO* pjlo = reinterpret_cast<JLO*>(pbasic);

	ActivateJlo(pjlo);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJloDEACTIVATE(BASIC* pbasic, int carg, CRef* aref)
{
	JLO* pjlo = reinterpret_cast<JLO*>(pbasic);

	DeactivateJlo();

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkDecoyDAMAGE(BASIC* pbasic, int carg, CRef* aref)
{
	DECOY* pdecoy = (DECOY*)(pbasic);

	JT* pjt = reinterpret_cast<JT*>(aref[0].m_pbasic);
	const int fForceDestroy = aref[1].RefCoerceS32().m_n;

	DamageJtDecoy(pdecoy, pjt, fForceDestroy);

	CRef refRet;
	refRet.SetTag(TAGK_Void);
	return refRet;
}

CRef RefThunkCameraENABLE(BASIC* pbasic, int carg, CRef* aref)
{
	CAMERA* pcamera = reinterpret_cast<CAMERA*>(pbasic);

	EnableCamera(pcamera);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkCameraDISABLE(BASIC* pbasic, int carg, CRef* aref)
{
	CAMERA* pcamera = reinterpret_cast<CAMERA*>(pbasic);

	DisableCamera(pcamera);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkEmitterPAUSE(BASIC* pbasic, int carg, CRef* aref)
{
	EMITTER* pemitter = reinterpret_cast<EMITTER*>(pbasic);

	float dt = 0.0f;

	if (carg > 0)
		dt = aref[0].RefCoerceF32().m_g;

	PauseEmitter(pemitter, dt);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkEmitterPAUSE_FOREVER(BASIC* pbasic, int carg, CRef* aref)
{
	EMITTER* pemitter = reinterpret_cast<EMITTER*>(pbasic);

	PauseEmitterIndefinite(pemitter);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkEmitterUNPAUSE(BASIC* pbasic, int carg, CRef* aref)
{
	EMITTER* pemitter = reinterpret_cast<EMITTER*>(pbasic);

	UnpauseEmitter(pemitter);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkEmitterADD_SKELETON(BASIC* pbasic, int carg, CRef* aref)
{
	EMITTER* pemitter = reinterpret_cast<EMITTER*>(pbasic);

	OID object = OID_Nil;
	OID otherObject = OID_Nil;
	float radius = 0.0f;
	float density = 0.0f;
	float radiusOther = 0.0f;
	float densityOther = 0.0f;

	if (carg > 0)
		object = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		otherObject = static_cast<OID>(aref[1].RefCoerceS32().m_n);

	if (carg > 2)
		radius = aref[2].RefCoerceF32().m_g;

	if (carg > 3)
		density = aref[3].RefCoerceF32().m_g;

	if (carg > 4)
		radiusOther = aref[4].RefCoerceF32().m_g;

	if (carg > 5)
		densityOther = aref[5].RefCoerceF32().m_g;

	AddEmitterSkeleton(pemitter, object, otherObject, radius, density, radiusOther, densityOther);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkLockgTRIGGER(BASIC* pbasic, int carg, CRef* aref)
{
	LOCKG* plockg = reinterpret_cast<LOCKG*>(pbasic);

	TriggerLockg(plockg);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkFlashPOS_WITHIN(BASIC* pbasic, int carg, CRef* aref)
{
	FLASH* pflash = reinterpret_cast<FLASH*>(pbasic);

	glm::vec3 pos(0.0f);

	if (carg > 0)
		pos = aref[0].m_vector;

	const int fWithin = FPosFlashWithin(pflash, &pos);

	CRef ret;
	ret.SetBool(fWithin);
	return ret;
}

CRef RefThunkBspPOINT_IN_BSP_QUICK(BASIC* pbasic, int carg, CRef* aref)
{
	SO* pso = reinterpret_cast<SO*>(pbasic);
	glm::vec3 pos(0.0f);

	if (carg > 0)
		pos = aref[0].m_vector;

	CRef ret;
	ret.SetBool(pso != nullptr && !pso->bspc.absp.empty() && PbspPointInBspQuick(&pos, pso->bspc.absp.data()) != nullptr);
	return ret;
}

CRef RefThunkWaypointSET_SM_GOAL(BASIC* pbasic, int carg, CRef* aref)
{
	WAYPOINT* pwaypoint = reinterpret_cast<WAYPOINT*>(pbasic);

	int trigger = 0;
	OID object = OID_Nil;
	OID stateMachine = OID_Nil;
	OID goal = OID_Nil;

	if (carg > 0)
		trigger = aref[0].RefCoerceS32().m_n;

	if (carg > 1)
		object = static_cast<OID>(aref[1].RefCoerceS32().m_n);

	if (carg > 2)
		stateMachine = static_cast<OID>(aref[2].RefCoerceS32().m_n);

	if (carg > 3)
		goal = static_cast<OID>(aref[3].RefCoerceS32().m_n);

	SetWaypointRsmg(pwaypoint, trigger, object, stateMachine, goal);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkDialogINSTRUCT(BASIC* pbasic, int carg, CRef* aref)
{
	DIALOG* pdialog = reinterpret_cast<DIALOG*>(pbasic);

	SetDialogInstruct(pdialog);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkDialogCONFRONT(BASIC* pbasic, int carg, CRef* aref)
{
	DIALOG* pdialog = reinterpret_cast<DIALOG*>(pbasic);

	SetDialogConfront(pdialog);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkDialogADD_EQUIVALENCE(BASIC* pbasic, int carg, CRef* aref)
{
	DIALOG* pdialog = reinterpret_cast<DIALOG*>(pbasic);

	OID oidWorld = OID_Nil;
	OID oidDialog = OID_Nil;

	if (carg > 0)
		oidWorld = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		oidDialog = static_cast<OID>(aref[1].RefCoerceS32().m_n);

	AddDialogEquivalence(pdialog, oidWorld, oidDialog);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkDialogTRIGGER(BASIC* pbasic, int carg, CRef* aref)
{
	DIALOG* pdialog = reinterpret_cast<DIALOG*>(pbasic);

	TriggerDialog(pdialog);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkDialogUNTRIGGER(BASIC* pbasic, int carg, CRef* aref)
{
	DIALOG* pdialog = reinterpret_cast<DIALOG*>(pbasic);

	UntriggerDialog(pdialog);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwSpawnRandomCoins(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	int count = 0;
	int value = 0;
	glm::vec3 pos{};

	if (carg > 0)
		count = aref[0].RefCoerceS32().m_n;

	if (carg > 1)
		value = aref[1].RefCoerceS32().m_n;

	if (carg > 2 && aref[2].m_tagk == TAGK_Vector)
		pos = aref[2].m_vector;

	SpawnRandomCoins(psw, count, value, &pos);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwSetDefaultReverb(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	REVERBK reverbk = static_cast<REVERBK>(0);
	int depth = 0;

	if (carg > 0)
		reverbk = static_cast<REVERBK>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		depth = aref[1].RefCoerceS32().m_n;

	SetSwDefaultReverb(psw, reverbk, depth);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwPUSH_REVERB(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	REVERBK reverbk = static_cast<REVERBK>(0);
	int depth = 0;

	if (carg > 0)
		reverbk = static_cast<REVERBK>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		depth = aref[1].RefCoerceS32().m_n;

	PushSwReverb(psw, reverbk, depth);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwPOP_REVERB(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	PopSwReverb(psw);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwLEVEL_VISITED(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	WID wid = static_cast<WID>(0);

	if (carg > 0)
		wid = static_cast<WID>(aref[0].RefCoerceS32().m_n);

	CRef ret;
	ret.SetBool(FLevelSwVisited(psw, wid));
	return ret;
}

CRef RefThunkSwLEVEL_PRIMARY(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	WID wid = static_cast<WID>(0);

	if (carg > 0)
		wid = static_cast<WID>(aref[0].RefCoerceS32().m_n);

	CRef ret;
	ret.SetBool(FLevelSwPrimary(psw, wid));
	return ret;
}

CRef RefThunkSwLEVEL_SECONDARY(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	WID wid = static_cast<WID>(0);

	if (carg > 0)
		wid = static_cast<WID>(aref[0].RefCoerceS32().m_n);

	CRef ret;
	ret.SetBool(FLevelSwSecondary(psw, wid));
	return ret;
}

CRef RefThunkSwLEVEL_TERTIARY(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	WID wid = static_cast<WID>(0);

	if (carg > 0)
		wid = static_cast<WID>(aref[0].RefCoerceS32().m_n);

	CRef ret;
	ret.SetBool(FLevelSwTertiary(psw, wid));
	return ret;
}

CRef RefThunkSwLEVEL_KEY_COLLECTED(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	int levelId = 0;

	if (carg > 0)
		levelId = aref[0].RefCoerceS32().m_n;

	CRef ret;
	ret.SetBool(FLevelKeyCollected(psw, levelId));
	return ret;
}

CRef RefThunkSwSECONDARY_TASK_AVAILABLE(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	int levelId = 0;

	if (carg > 0)
		levelId = aref[0].RefCoerceS32().m_n;

	CRef ret;
	ret.SetBool(FSecondaryTaskAvailable(psw, levelId));
	return ret;
}

CRef RefThunkSwCOMMENTARY_UNLOCKED_FOR_LEVEL(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	int levelId = 0;

	if (carg > 0)
		levelId = aref[0].RefCoerceS32().m_n;

	CRef ret;
	ret.SetBool(FCommentaryUnlockedForLevel(psw, levelId));
	return ret;
}

CRef RefThunkSwSET_LEVEL_DATA_VALUE(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);
	uint32_t worldLevelKey = UINT32_MAX;
	int key = 0;
	int value = 0;

	if (carg > 0)
		worldLevelKey = static_cast<uint32_t>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		key = aref[1].RefCoerceS32().m_n;

	if (carg > 2)
		value = aref[2].RefCoerceS32().m_n;

	SetSwLevelDataValue(psw, worldLevelKey, key, value);
	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwGET_LEVEL_DATA_VALUE(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);
	uint32_t worldLevelKey = UINT32_MAX;
	int key = 0;

	if (carg > 0)
		worldLevelKey = static_cast<uint32_t>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		key = aref[1].RefCoerceS32().m_n;

	CRef ret;
	ret.SetS32(GetSwLevelDataValue(psw, worldLevelKey, key));
	return ret;
}

CRef RefThunkSwWORLD_VISITED(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	int world = 0;

	if (carg > 0)
		world = aref[0].RefCoerceS32().m_n;

	CRef ret;
	ret.SetBool(FWorldVisited(psw, world));
	return ret;
}

CRef RefThunkSwWORLD_COMPLETE(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	int world = 0;

	if (carg > 0)
		world = aref[0].RefCoerceS32().m_n;

	CRef ret;
	ret.SetBool(FWorldComplete(psw, world));
	return ret;
}

CRef RefThunkSwRECALCULATE_PERCENT_COMPLETION(BASIC* pbasic, int carg, CRef* aref)
{
	CRef ret;
	ret.SetS32(RecalculatePercentCompletion());
	return ret;
}

CRef RefThunkSwCANCEL_DIALOG(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	CancelSwDialogPlaying(psw);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwSET_SMA_PROMPT(BASIC* pbasic, int carg, CRef* aref)
{
	PRK prk = static_cast<PRK>(0);
	SMA* psma = nullptr;

	if (carg > 0)
		prk = static_cast<PRK>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		psma = reinterpret_cast<SMA*>(aref[1].m_pbasic);

	SetSmaPrompt(prk, psma);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwACQUIRE_LETTERBOX(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	AcquireSwLetterbox(psw);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwRELEASE_LETTERBOX(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	ReleaseSwLetterbox(psw);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwSET_EXCITEMENT(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	IEXC iexc = static_cast<IEXC>(0);

	if (carg > 0)
		iexc = static_cast<IEXC>(aref[0].RefCoerceS32().m_n);

	EXC* pexc = SetExcitement(psw, iexc);

	CRef ret;
	ret.SetBasic(reinterpret_cast<BASIC*>(pexc));
	return ret;
}

CRef RefThunkSwUNSET_EXCITEMENT(BASIC* pbasic, int carg, CRef* aref)
{
	EXC* pexc = nullptr;

	if (carg > 0)
		pexc = reinterpret_cast<EXC*>(aref[0].m_pbasic);

	UnsetExcitementSw(pexc);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwSET_MUSIC_REGISTER(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);
	if (carg >= 2)
		SetSwMusicRegister(psw, aref[0].RefCoerceS32().m_n,
			static_cast<byte>(aref[1].RefCoerceS32().m_n));

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwREFRESH_MIDI_REGISTER(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);
	if (carg >= 2)
		RefreshSwMidiRegister(psw, aref[0].RefCoerceS32().m_n);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwRESPONSE_TEXT(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	RESPK respk = static_cast<RESPK>(0);

	if (carg > 0)
		respk = static_cast<RESPK>(aref[0].RefCoerceS32().m_n);

	CRef ret;
	ret.SetS32((signed int)GetResponseText(psw, respk));
	return ret;
}

CRef RefThunkSwHANDS_OFF(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	IncrementSwHandsOff(psw);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwHANDS_ON(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	DecrementSwHandsOff(psw);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwLOCK_VAULT(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	int vault = 0;

	if (carg > 0)
		vault = aref[0].RefCoerceS32().m_n;

	SetSwVaultLocked(psw, vault);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSwUNLOCK_VAULT(BASIC* pbasic, int carg, CRef* aref)
{
	SW* psw = reinterpret_cast<SW*>(pbasic);

	int vault = 0;

	if (carg > 0)
		vault = aref[0].RefCoerceS32().m_n;

	UnlockSwVault(psw, vault);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}


CRef RefThunkCmPUSH_LOOK_KIND(BASIC* pbasic, int carg, CRef* aref)
{
	CM* pcm = static_cast<CM*>(pbasic);
	LOOKK lookk = static_cast<LOOKK>(aref[0].RefCoerceS32().m_n);

	PushCmLookk(pcm, lookk);

	CRef ref;
	ref.SetTag(TAGK_Void);
	return ref;
}

CRef RefThunkCmPOP_LOOK_KIND(BASIC* pbasic, int carg, CRef* aref)
{
	CM* pcm = static_cast<CM*>(pbasic);

	CRef ref;
	ref.SetS32(static_cast<int>(LookkPopCm(pcm)));
	return ref;
}

CRef RefThunkCmLOOK_KIND(BASIC* pbasic, int carg, CRef* aref)
{
	CM* pcm = static_cast<CM*>(pbasic);

	CRef ref;
	ref.SetS32(static_cast<int>(LookkCurCm(pcm)));
	return ref;
}

CRef RefThunkCmSET_SNIPER_FOCUS(BASIC* pbasic, int carg, CRef* aref)
{
	CM* pcm = static_cast<CM*>(pbasic);
	ALO* paloFocus = static_cast<ALO*>(aref[0].m_pbasic);
	float sRadius = aref[1].RefCoerceF32().m_g;
	float rScreen = aref[2].RefCoerceF32().m_g;

	SetCmSniperFocus(pcm, paloFocus, sRadius, rScreen);

	CRef ref;
	ref.SetTag(TAGK_Void);
	return ref;
}

CRef RefThunkCmJOLT(BASIC* pbasic, int carg, CRef* aref)
{
	CM* pcm = static_cast<CM*>(pbasic);

	JoltCm(pcm);

	CRef ref;
	ref.SetTag(TAGK_Void);
	return ref;
}

CRef RefThunkCmCUT(BASIC* pbasic, int carg, CRef* aref)
{
	CM* pcm = static_cast<CM*>(pbasic);

	CutCm(pcm);

	CRef ref;
	ref.SetTag(TAGK_Void);
	return ref;
}

CRef RefThunkCmVISIBLE_SPHERE(BASIC* pbasic, int carg, CRef* aref)
{
	CM* pcm = static_cast<CM*>(pbasic);
	const glm::vec3& pos = aref[0].m_vector;
	float sRadius = aref[1].RefCoerceF32().m_g;
	float sMRD = aref[2].RefCoerceF32().m_g;
	float uAlpha = 0.0f;

	bool fVisible = FVisibleCmSphere(pcm, pos, sRadius, sMRD, uAlpha);

	CRef ref;
	ref.SetBool(fVisible);
	return ref;
}

CRef RefThunkWarpSET_SM_GOAL(BASIC* pbasic, int carg, CRef* aref)
{
	WARP* pwarp = static_cast<WARP*>(pbasic);

	const int fOnTrigger = aref[0].RefCoerceS32().m_n;
	const OID oidRoot = static_cast<OID>(aref[1].RefCoerceS32().m_n);
	const OID oidSM = static_cast<OID>(aref[2].RefCoerceS32().m_n);
	const OID oidGoal = static_cast<OID>(aref[3].RefCoerceS32().m_n);

	SetWarpRsmg(pwarp, fOnTrigger, oidRoot, oidSM, oidGoal);

	CRef ref;
	ref.SetTag(TAGK_Void);
	return ref;
}

CRef RefThunkWarpTRIGGER(BASIC* pbasic, int carg, CRef* aref)
{
	WARP* pwarp = static_cast<WARP*>(pbasic);

	TriggerWarp(pwarp);

	CRef ref;
	ref.SetTag(TAGK_Void);
	return ref;
}

CRef RefThunkExplsEXPLODE(BASIC* pbasic, int carg, CRef* aref)
{
	EXPL* pexpl = static_cast<EXPL*>(pbasic);

	ExplodeExpl(pexpl);

	CRef ref;
	ref.SetTag(TAGK_Void);
	return ref;
}

CRef RefThunkExplsEXPLODE_OVR(BASIC* pbasic, int carg, CRef* aref)
{
	EXPL* pexpl = static_cast<EXPL*>(pbasic);

	const int grfexplso = static_cast<int>(aref[0].RefCoerceS32().m_n);
	ALO* paloReference = static_cast<ALO*>(aref[1].m_pbasic);
	glm::vec3 normal = aref[2].m_vector;
	glm::vec3 posOrigin = aref[3].m_vector;
	const float rScale = aref[4].RefCoerceF32().m_g;
	const float sRadius = aref[5].RefCoerceF32().m_g;

	ExplodeExplParams(pexpl, grfexplso, paloReference, &normal, &posOrigin, rScale, sRadius);

	CRef ref;
	ref.SetTag(TAGK_Void);
	return ref;
}

CRef RefThunkExploADD_SKELETON(BASIC* pbasic, int carg, CRef* aref)
{
	EXPLO* pexplo = (EXPLO*)pbasic;

	const OID oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);
	const OID oidOther = static_cast<OID>(aref[1].RefCoerceS32().m_n);
	const float sRadius = aref[2].RefCoerceF32().m_g;
	const float gDensity = aref[3].RefCoerceF32().m_g;
	const float sRadiusOther = aref[4].RefCoerceF32().m_g;
	const float gDensityOther = aref[5].RefCoerceF32().m_g;

	AddExploSkeleton(pexplo, oid, oidOther, sRadius, gDensity, sRadiusOther, gDensityOther);

	CRef ref;
	ref.SetTag(TAGK_Void);
	return ref;
}

CRef RefThunkExitTRIGGER(BASIC* pbasic, int carg, CRef* aref)
{
	EXIT* pexit = reinterpret_cast<EXIT*>(pbasic);

	TriggerExit(pexit);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkVolCHECK_POINT(BASIC* pbasic, int carg, CRef* aref)
{
	VOL* pvol = (VOL*)pbasic;
	glm::vec3 pos = aref[0].m_vector;

	CRef ref;
	ref.SetBool(FCheckVolPoint(pvol, &pos));
	return ref;
}

CRef RefThunkAsegAPPLY(BASIC* pbasic, int carg, CRef* aref)
{
	CRef refRet;
	refRet.SetBasic(nullptr);

	if (pbasic == nullptr)
		return refRet;

	ASEG* paseg = reinterpret_cast<ASEG*>(pbasic);

	ALO* paloRoot = nullptr;
	float t = 0.0f;
	float svt = 1.0f;
	FAPL fapl = FAPL_Nil;

	if (carg > 0 && aref != nullptr)
	{
		if (aref[0].m_tagk == TAGK_Basic)
			paloRoot = reinterpret_cast<ALO*>(aref[0].m_pbasic);
		else if (aref[0].m_tagk != TAGK_Nil && aref[0].m_tagk != TAGK_None)
			std::printf("ASEG APPLY WARNING: invalid root tag=%d\n", static_cast<int>(aref[0].m_tagk));
	}

	if (carg > 1)
		t = aref[1].RefCoerceF32().m_g;

	if (carg > 2)
		svt = aref[2].RefCoerceF32().m_g;

	if (carg > 3)
		fapl = static_cast<FAPL>(aref[3].RefCoerceS32().m_n);

	ASEGA* pasega = PasegaApplyAseg(paseg, paloRoot, t, svt, fapl);
	refRet.SetBasic(reinterpret_cast<BASIC*>(pasega));
	return refRet;
}

CRef RefThunkAsegENSURE(BASIC* pbasic, int carg, CRef* aref)
{
	ASEG* paseg = reinterpret_cast<ASEG*>(pbasic);

	ALO* paloRoot = nullptr;
	SEEK seek = SEEK_Nil;
	float t = 0.0f;
	float svt = 1.0f;
	FAPL fapl = FAPL_Nil;

	if (carg > 0) {
		paloRoot = reinterpret_cast<ALO*>(aref[0].m_pbasic);
	}

	if (carg > 1) {
		seek = static_cast<SEEK>(aref[1].RefCoerceS32().m_n);
	}

	if (carg > 2) {
		t = aref[2].RefCoerceF32().m_g;
	}

	if (carg > 3) {
		svt = aref[3].RefCoerceF32().m_g;
	}

	if (carg > 4) {
		fapl = static_cast<FAPL>(aref[4].RefCoerceS32().m_n);
	}

	ASEGA* pasega = PasegaEnsureAseg(paseg, paloRoot, seek, t, svt, fapl);

	CRef ret;
	ret.SetBasic(reinterpret_cast<BASIC*>(pasega));
	return ret;
}

CRef RefThunkAsegFIND_LABEL(BASIC* pbasic, int carg, CRef* aref)
{
	ASEG* paseg = reinterpret_cast<ASEG*>(pbasic);

	OID oid = OID_Nil;

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	float t = TFindAsegLabel(paseg, oid);

	CRef ret;
	ret.SetF32(t);
	return ret;
}

CRef RefThunkSmAPPLY(BASIC* pbasic, int carg, CRef* aref)
{
	SM* psm = reinterpret_cast<SM*>(pbasic);

	ALO* paloRoot = static_cast<ALO*>(aref[0].m_pbasic);
	const OID oidInitialState = static_cast<OID>(aref[1].RefCoerceS32().m_n);
	const FAPL fapl = static_cast<FAPL>(aref[2].RefCoerceS32().m_n);

	SMA* psma = PsmaApplySm(psm, paloRoot, oidInitialState, fapl);

	CRef refRet;
	refRet.SetBasic(reinterpret_cast<BASIC*>(psma));
	return refRet;
}

CRef RefThunkRchmSET_NATURAL_COEFFICIENTS(BASIC* pbasic, int cref, CRef* aref)
{
	RCHM* prchm = reinterpret_cast<RCHM*>(pbasic);

	const float rcl = aref[0].RefCoerceF32().m_g;
	const float io = aref[1].RefCoerceF32().m_g;
	const float lhub = aref[2].RefCoerceF32().m_g;

	SetRchmNaturalCoefficients(prchm, rcl, io, lhub);

	CRef refRet;
	refRet.SetTag(TAGK_Void);
	return refRet;
}

CRef RefThunkRchmSET_CENTER_COEFFICIENTS(BASIC* pbasic, int cref, CRef* aref)
{
	RCHM* prchm = reinterpret_cast<RCHM*>(pbasic);

	const float rcl = aref[0].RefCoerceF32().m_g;
	const float io = aref[1].RefCoerceF32().m_g;
	const float lhub = aref[2].RefCoerceF32().m_g;

	SetRchmCenterCoefficients(prchm, rcl, io, lhub);

	CRef refRet;
	refRet.SetTag(TAGK_Void);
	return refRet;
}

CRef RefThunkRwmADD_AMMO(BASIC* pbasic, int cref, CRef* aref)
{
	RWM* prwm = reinterpret_cast<RWM*>(pbasic);

	const OID oidCache = static_cast<OID>(aref[0].RefCoerceS32().m_n);
	const int cpso = aref[1].RefCoerceS32().m_n;
	const bool fReuse = aref[2].RefCoerceS32().m_n != 0;
	const bool fEnabled = aref[3].RefCoerceS32().m_n != 0;

	AddRwmCache(prwm, oidCache, cpso, fReuse, fEnabled);

	CRef refRet;
	refRet.SetTag(TAGK_Void);
	return refRet;
}

CRef RefThunkRwmENABLE_CACHE(BASIC* pbasic, int cref, CRef* aref)
{
	RWM* prwm = reinterpret_cast<RWM*>(pbasic);
	const OID oidCache = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	EnableRwmCache(prwm, oidCache);

	CRef refRet;
	refRet.SetTag(TAGK_Void);
	return refRet;
}

CRef RefThunkRwmDISABLE_CACHE(BASIC* pbasic, int cref, CRef* aref)
{
	RWM* prwm = reinterpret_cast<RWM*>(pbasic);
	const OID oidCache = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	DisableRwmCache(prwm, oidCache);

	CRef refRet;
	refRet.SetTag(TAGK_Void);
	return refRet;
}

CRef RefThunkRwmRESIZE_CACHE(BASIC* pbasic, int cref, CRef* aref)
{
	RWM* prwm = reinterpret_cast<RWM*>(pbasic);

	const OID oidCache = static_cast<OID>(aref[0].RefCoerceS32().m_n);
	const int cpso = aref[1].RefCoerceS32().m_n;

	ResizeRwmCache(prwm, oidCache, cpso);

	CRef refRet;
	refRet.SetTag(TAGK_Void);
	return refRet;
}

CRef RefThunkRwmRELOAD(BASIC* pbasic, int cref, CRef* aref)
{
	RWM* prwm = reinterpret_cast<RWM*>(pbasic);

	ReloadRwm(prwm);

	CRef refRet;
	refRet.SetTag(TAGK_Void);
	return refRet;
}

CRef RefThunkRwmFIRE(BASIC* pbasic, int cref, CRef* aref)
{
	RWM* prwm = reinterpret_cast<RWM*>(pbasic);
	const bool fAim = aref[0].RefCoerceS32().m_n != 0;

	const bool fFired = FFireRwm(prwm, fAim);

	CRef refRet;
	refRet.SetBool(fFired);
	return refRet;
}

CRef RefThunkRwmCLEAR_FIRE_INFO(BASIC* pbasic, int cref, CRef* aref)
{
	RWM* prwm = reinterpret_cast<RWM*>(pbasic);

	ClearRwmFireInfo(prwm);

	CRef refRet;
	refRet.SetTag(TAGK_Void);
	return refRet;
}

CRef RefThunkRwmCLEAR_TARGET_INFO(BASIC* pbasic, int cref, CRef* aref)
{
	RWM* prwm = reinterpret_cast<RWM*>(pbasic);

	ClearRwmTargetInfo(prwm);

	CRef refRet;
	refRet.SetTag(TAGK_Void);
	return refRet;
}

CRef RefThunkRwmCLEAR_AIM_CONSTRAINTS(BASIC* pbasic, int cref, CRef* aref)
{
	RWM* prwm = reinterpret_cast<RWM*>(pbasic);

	ClearRwmAimControl(prwm);

	CRef refRet;
	refRet.SetTag(TAGK_Void);
	return refRet;
}

CRef RefThunkWrADD_CIRCLE_WARP(BASIC* pbasic, int cref, CRef* aref)
{
	CRef ret;

	WR *pwr = reinterpret_cast <WR*>(pbasic);

	AddWrCircleWarp(pwr);

	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkWrADD_BEND_WARP(BASIC* pbasic, int carg, CRef* aref)
{
	WR *pwr = reinterpret_cast<WR*>(pbasic);

	AddWrBendWarp(pwr);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkWrADD_BEND_NOISE(BASIC* pbasic, int carg, CRef* aref)
{
	WR* pwr = static_cast<WR*>(pbasic);

	float amplitude = 0.0f;
	float frequency = 0.0f;
	float phase = 0.0f;
	float randomness = 0.0f;

	if (carg > 0)
		amplitude = aref[0].RefCoerceF32().m_g;

	if (carg > 1)
		frequency = aref[1].RefCoerceF32().m_g;

	if (carg > 2)
		phase = aref[2].RefCoerceF32().m_g;

	if (carg > 3)
		randomness = aref[3].RefCoerceF32().m_g;

	AddWrBendNoise(pwr, amplitude, frequency, phase, randomness);

	CRef ref;
	ref.SetTag(TAGK_Void);
	return ref;
}

CRef RefThunkWrADD_SWIVEL_NOISE(BASIC* pbasic, int carg, CRef* aref)
{
	WR* pwr = reinterpret_cast<WR*>(pbasic);

	float amplitude = 0.0f;
	float frequency = 0.0f;
	float phase = 0.0f;
	float randomness = 0.0f;

	if (carg > 0) amplitude = aref[0].RefCoerceF32().m_g;
	if (carg > 1) frequency = aref[1].RefCoerceF32().m_g;
	if (carg > 2) phase = aref[2].RefCoerceF32().m_g;
	if (carg > 3) randomness = aref[3].RefCoerceF32().m_g;

	AddWrSwivelNoise(pwr, amplitude, frequency, phase, randomness);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgCLEAR(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);

	ClearJsg(pjsg);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgAPPLY(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);
	JT* pjt = nullptr;

	if (carg > 0)
		pjt = reinterpret_cast<JT*>(aref[0].m_pbasic);

	ApplyJsg(pjsg, pjt);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgRETRACT(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);

	RetractJsg(pjsg);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgCONTEXT(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);
	OID oid = OID_Nil;

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	AddJsgContext(pjsg, oid);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgCUT(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);
	OID oid = OID_Nil;

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	AddJsgCut(pjsg, oid);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgFOCUS(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);
	OID oid = OID_Nil;

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	AddJsgFocus(pjsg, oid);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgLABEL(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);
	OID oid = OID_Nil;

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	AddJsgLabel(pjsg, oid);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgTUNNEL(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);
	OID oid = OID_Nil;

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	AddJsgTunnel(pjsg, oid);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgPAUSE(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);
	OID oid = OID_Nil;
	float dtPause = 0.0f;

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		dtPause = aref[1].RefCoerceF32().m_g;

	AddJsgPause(pjsg, oid, dtPause);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgJUMP(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);
	OID oid = OID_Nil;
	JSGJK jsgjk = JSGJK_Fall;
	JSGLK jsglk = JSGLK_Normal;
	int fAsync = 0;

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		jsgjk = static_cast<JSGJK>(aref[1].RefCoerceS32().m_n);

	if (carg > 2)
		jsglk = static_cast<JSGLK>(aref[2].RefCoerceS32().m_n);

	if (carg > 3)
		fAsync = aref[3].m_tagk == TAGK_Bool ? aref[3].m_bool : aref[3].RefCoerceS32().m_n;

	AddJsgJump(pjsg, oid, jsgjk, jsglk, fAsync);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgRUN(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);
	OID oid = OID_Nil;
	float uRun = 0.0f;
	int fAsync = 0;

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		uRun = aref[1].RefCoerceF32().m_g;

	if (carg > 2)
		fAsync = aref[2].m_tagk == TAGK_Bool ? aref[2].m_bool : aref[2].RefCoerceS32().m_n;

	AddJsgRun(pjsg, oid, uRun, fAsync);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgCLIMB(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);
	OID oid = OID_Nil;
	float uClimb = 0.0f;
	int fAsync = 0;

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		uClimb = aref[1].RefCoerceF32().m_g;

	if (carg > 2)
		fAsync = aref[2].m_tagk == TAGK_Bool ? aref[2].m_bool : aref[2].RefCoerceS32().m_n;

	AddJsgClimb(pjsg, oid, uClimb, fAsync);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgATTACK(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);
	OID oid = OID_Nil;
	FTAK ftak = (FTAK)0;
	int fAsync = 0;

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		ftak = static_cast<FTAK>(aref[1].RefCoerceS32().m_n);

	if (carg > 2)
		fAsync = aref[2].m_tagk == TAGK_Bool ? aref[2].m_bool : aref[2].RefCoerceS32().m_n;

	AddJsgAttack(pjsg, oid, ftak, fAsync);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgASEG(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);
	OID oid = OID_Nil;
	int fAsync = 0;

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		fAsync = aref[1].m_tagk == TAGK_Bool ? aref[1].m_bool : aref[1].RefCoerceS32().m_n;

	AddJsgAseg(pjsg, oid, fAsync);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgCLOCK(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);
	float rdt = 1.0f;

	if (carg > 0)
		rdt = aref[0].RefCoerceF32().m_g;

	AddJsgClock(pjsg, rdt);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkJsgHIDE(BASIC* pbasic, int carg, CRef* aref)
{
	JSG* pjsg = reinterpret_cast<JSG*>(pbasic);

	AddJsgHide(pjsg);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkRubySET_RYTHM_SEQUENCE(BASIC* pbasic, int carg, CRef* aref)
{
	RUBY* pruby = reinterpret_cast<RUBY*>(pbasic);
	RYTHMSEQUENCE* prythmSequence = nullptr;

	if (carg > 0)
		prythmSequence =
		reinterpret_cast<RYTHMSEQUENCE*>(aref[0].m_pbasic);

	SetRubyPendingRythm(pruby, prythmSequence);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkRubyCLEAR_RYTHM_SEQUENCE(BASIC* pbasic, int carg, CRef* aref)
{
	RUBY* pruby = reinterpret_cast<RUBY*>(pbasic);

	ClearRubyRythmSequence(pruby);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkRubyADD_RYTHM_EVENT(BASIC* pbasic, int carg, CRef* aref)
{
	RUBY* pruby = reinterpret_cast<RUBY*>(pbasic);

	ASEGA* pasega = nullptr;
	int cBeats = 0;

	if (carg > 0)
		pasega = reinterpret_cast<ASEGA*>(aref[0].m_pbasic);

	if (carg > 1)
		cBeats = aref[1].RefCoerceS32().m_n;

	AddRubyRythmEvent(pruby, pasega, cBeats);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkRubyREMOVE_RYTHM_EVENT(BASIC* pbasic, int carg, CRef* aref)
{
	RUBY* pruby = reinterpret_cast<RUBY*>(pbasic);
	ASEGA* pasega = nullptr;

	if (carg > 0)
		pasega = reinterpret_cast<ASEGA*>(aref[0].m_pbasic);

	RemoveRubyRythmEvent(pruby, pasega);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkRubyDAMAGE_BOSS(BASIC* pbasic, int carg, CRef* aref)
{
	RUBY* pruby = reinterpret_cast<RUBY*>(pbasic);

	DamageRubyBoss(pruby);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkRythmSequenceADD_MATCH(BASIC* pbasic, int carg, CRef* aref)
{
	RYTHMSEQUENCE* prythmSequence = reinterpret_cast<RYTHMSEQUENCE*>(pbasic);

	int iMeasure = 0;
	float tBeat = 0.0f;
	int iWeapon = 0;

	if (carg > 0)
		iMeasure = aref[0].RefCoerceS32().m_n;

	if (carg > 1)
		tBeat = aref[1].RefCoerceF32().m_g;

	if (carg > 2)
		iWeapon = aref[2].RefCoerceS32().m_n;

	AddRythmSequenceMatch(prythmSequence, iMeasure, tBeat, iWeapon);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkRythmSequenceADD_SPLICE_EVENT(BASIC* pbasic, int carg, CRef* aref)
{
	RYTHMSEQUENCE* prythmSequence = reinterpret_cast<RYTHMSEQUENCE*>(pbasic);

	int iMeasure = 0;
	float tBeat = 0.0f;
	int iEvent = 0;

	if (carg > 0)
		iMeasure = aref[0].RefCoerceS32().m_n;

	if (carg > 1)
		tBeat = aref[1].RefCoerceF32().m_g;

	if (carg > 2)
		iEvent = aref[2].RefCoerceS32().m_n;

	AddRythmSequenceSpliceEvent(prythmSequence, iMeasure, tBeat, iEvent);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkRythmSequenceENABLE_INPUT(BASIC* pbasic, int carg, CRef* aref)
{
	RYTHMSEQUENCE* prythmSequence = reinterpret_cast<RYTHMSEQUENCE*>(pbasic);

	int iMeasure = 0;
	float tBeat = 0.0f;

	if (carg > 0)
		iMeasure = aref[0].RefCoerceS32().m_n;

	if (carg > 1)
		tBeat = aref[1].RefCoerceF32().m_g;

	AddRythmSequenceEnableInput(prythmSequence, iMeasure, tBeat);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkRythmSequenceDISABLE_INPUT(BASIC* pbasic, int carg, CRef* aref)
{
	RYTHMSEQUENCE* prythmSequence = reinterpret_cast<RYTHMSEQUENCE*>(pbasic);

	int iMeasure = 0;
	float tBeat = 0.0f;

	if (carg > 0)
		iMeasure = aref[0].RefCoerceS32().m_n;

	if (carg > 1)
		tBeat = aref[1].RefCoerceF32().m_g;

	AddRythmSequenceDisableInput(prythmSequence, iMeasure, tBeat);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkRythmSequenceENABLE_EFFECT(BASIC* pbasic, int carg, CRef* aref)
{
	RYTHMSEQUENCE* prythmSequence = reinterpret_cast<RYTHMSEQUENCE*>(pbasic);

	int iMeasure = 0;
	float tBeat = 0.0f;

	if (carg > 0)
		iMeasure = aref[0].RefCoerceS32().m_n;

	if (carg > 1)
		tBeat = aref[1].RefCoerceF32().m_g;

	AddRythmSequenceEnableEffect(prythmSequence, iMeasure, tBeat);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkRythmSequenceDISABLE_EFFECT(BASIC* pbasic, int carg, CRef* aref)
{
	RYTHMSEQUENCE* prythmSequence = reinterpret_cast<RYTHMSEQUENCE*>(pbasic);

	int iMeasure = 0;
	float tBeat = 0.0f;

	if (carg > 0)
		iMeasure = aref[0].RefCoerceS32().m_n;

	if (carg > 1)
		tBeat = aref[1].RefCoerceF32().m_g;

	AddRythmSequenceDisableEffect(prythmSequence, iMeasure, tBeat);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkMgvAPPLY_DEATH_THROW(BASIC* pbasic, int carg, CRef* aref)
{
	MGV* pmgv = reinterpret_cast<MGV*>(pbasic);
	LO* ploSource = carg > 0 ? reinterpret_cast<LO*>(aref[0].m_pbasic) : nullptr;
	ApplyMgvDeathThrow(pmgv, ploSource);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkMgvADD_RESPAWN_POINT(BASIC* pbasic, int carg, CRef* aref)
{
	MGV* pmgv = reinterpret_cast<MGV*>(pbasic);
	PNT* ppnt = carg > 0 ? reinterpret_cast<PNT*>(aref[0].m_pbasic) : nullptr;
	AddMgvRespawnPoint(pmgv, ppnt);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkMgvRESPAWN_OBJECT(BASIC* pbasic, int carg, CRef* aref)
{
	MGV* pmgv = reinterpret_cast<MGV*>(pbasic);
	LO* plo = carg > 0 ? reinterpret_cast<LO*>(aref[0].m_pbasic) : nullptr;
	RespawnMgvObject(pmgv, plo);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAsegaSEEK(BASIC* pbasic, int carg, CRef* aref)
{
	ASEGA* pasega = reinterpret_cast<ASEGA*>(pbasic);

	SEEK seek = SEEK_Nil;
	float tLocal = 0.0f;
	float svt = 1.0f;

	if (carg > 0)
		seek = static_cast<SEEK>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		tLocal = aref[1].RefCoerceF32().m_g;

	if (carg > 2)
		svt = aref[2].RefCoerceF32().m_g;

	SeekAsega(pasega, seek, tLocal, svt);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAsegaRETRACT(BASIC* pbasic, int carg, CRef* aref)
{
	ASEGA* pasega = reinterpret_cast<ASEGA*>(pbasic);

	RetractAsega(pasega);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAsegaSNAP(BASIC* pbasic, int carg, CRef* aref)
{
	ASEGA* pasega = reinterpret_cast<ASEGA*>(pbasic);

	int fForce = 0;

	if (carg > 0)
		fForce = aref[0].RefCoerceS32().m_n;

	SnapAsega(pasega, fForce);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkAsegaRETRACT_ACTSEG(BASIC* pbasic, int carg, CRef* aref)
{
	ASEGA* pasega = reinterpret_cast<ASEGA*>(pbasic);

	OID oid = OID_Nil;
	GRFRA grfra = static_cast<GRFRA>(0);

	if (carg > 0)
		oid = static_cast<OID>(aref[0].RefCoerceS32().m_n);

	if (carg > 1)
		grfra = static_cast<GRFRA>(aref[1].RefCoerceS32().m_n);

	RetractAsegaActseg(pasega, oid, grfra);

	CRef ret;
	ret.SetTag(TAGK_Void);
	return ret;
}

CRef RefThunkSmaRETRACT(BASIC* pbasic, int carg, CRef* aref)
{
	SMA* psma = reinterpret_cast<SMA*>(pbasic);

	RetractSma(psma);

	CRef refRet;
	refRet.SetTag(TAGK_Void);
	return refRet;
}

EOPID g_aeopid[1179];
