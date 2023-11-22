#include "erase.h"

//void DeleteObject(LO* plo)
//{
//	switch (plo->pvtlo->cid)
//	{
//		case CID_LO:
//			DeleteLo(plo);
//		break;
//
//		case CID_ALO:
//			DeleteAlo(plo);
//		break;
//
//		case CID_SO:
//			DeleteSo(plo);
//		break;
//
//		case CID_MS:
//			DeleteSo(plo);
//		break;
//
//		case CID_PO:
//			DeletePo(plo);
//		break;
//
//		case CID_STEP:
//			DeleteStep(plo);
//		break;
//
//		case CID_JT:
//			DeleteJt(plo);
//		break;
//		
//		case CID_STEPGUARD:
//			DeleteStepguard(plo);
//		break;
//
//		case CID_SMARTGUARD:
//			DeleteSmartGuard(plo);
//		break;
//
//		case CID_GOMER:
//			DeleteGomer(plo);
//		break;
//
//		case CID_UBG:
//			DeleteUbg(plo);
//		break;
//
//		case CID_MBG:
//			DeleteMbg(plo);
//		break;
//
//		case CID_BHG:
//			DeleteBhg(plo);
//		break;
//
//		case CID_MURRAY:
//			DeleteMurray(plo);
//		break;
//
//		case CID_PUFFC:
//			DeletePuffc(plo);
//		break;
//
//		case CID_CRFOD:
//			DeleteCrfod(plo);
//		break;
//
//		case CID_CRFODB:
//			DeleteCrfodb(plo);
//		break;
//
//		case CID_CRFODK:
//			DeleteCrfodk(plo);
//		break;
//
//		case CID_TANK:
//			DeleteTank(plo);
//		break;
//
//		case CID_JP:
//			DeleteJp(plo);
//		break;
//
//		case CID_HG:
//			DeleteHg(plo);
//		break;
//
//		case CID_MECHA:
//			DeleteMecha(plo);
//		break;
//
//		case CID_ROV:
//			DeleteRov(plo);
//		break;
//
//		case CID_TURRET:
//			DeleteTurret(plo);
//		break;
//
//		case CID_VAULT:
//			DeleteVault(plo);
//		break;
//
//		case CID_PUFFER:
//			DeletePuffer(plo);
//		break;
//
//		case CID_MGV:
//			//DeleteMGV(plo);
//		break;
//
//		case CID_SUV:
//			//DeleteSUV(plo);
//		break;
//
//		case CID_CYCLE:
//			DeleteCycle(plo);
//		break;
//
//		case CID_LGN:
//			DeleteLgn(plo);
//		break;
//
//		case CID_JACK:
//			DeleteJack(plo);
//		break;
//
//		case CID_RIPG:
//			DeleteRipg(plo);
//		break;
//
//		case CID_WATER:
//			DeleteWater(plo);
//		break;
//
//		case CID_BRK:
//			DeleteBrk(plo);
//		break;
//
//		case CID_BREAK:
//			DeleteBreak(plo);
//		break;
//
//		case CID_ALBRK:
//			DeleteAlbrk(plo);
//		break;
//
//		case CID_CAN:
//			DeleteCan(plo);
//		break;
//
//		case CID_DARTGUN:
//			//DeleteDartGun(plo);
//		break;
//		
//		case CID_SWP:
//			DeleteSwp(plo);
//		break;
//
//		case CID_FRAGILE:
//			DeleteFragile(plo);
//		break;
//
//		case CID_ZAPBREAK:
//			//DeleteZapBreak(plo);
//		break;
//
//		case CID_BRKP:
//			DeleteBrkp(plo);
//		break;
//
//		case CID_BUTTON:
//			DeleteButton(plo);
//		break;
//
//		case CID_VOLBTN:
//			DeleteVolbtn(plo);
//		break;
//
//		case CID_JLOVOL:
//			DeleteJlovol(plo);
//		break;
//
//		case CID_SQUISH:
//			DeleteSquish(plo);
//		break;
//
//		case CID_BONE:
//			DeleteBone(plo);
//		break;
//
//		case CID_SPRIZE:
//			DeleteSprize(plo);
//		break;
//
//		case CID_SCPRIZE:
//			DeleteScprize(plo);
//		break;
//
//		case CID_LIFETKN:
//			DeleteLifetkn(plo);
//		break;
//
//		case CID_CLUE:
//			DeleteClue(plo);
//		break;
//
//		case CID_ALARM:
//			DeleteAlarm(plo);
//		break;
//
//		case CID_SENSOR:
//			DeleteSensor(plo);
//		break;
//
//		case CID_LASEN:
//			DeleteLasen(plo);
//		break;
//
//		case CID_CAMSEN:
//			DeleteCamsen(plo);
//		break;
//
//		case CID_PRSEN:
//			DeletePrsen(plo);
//		break;
//
//		case CID_BARRIER:
//			DeleteBarrier(plo);
//		break;
//
//		case CID_IKH:
//			DeleteIkh(plo);
//		break;
//
//		case CID_TZP:
//			DeleteTzp(plo);
//		break;
//
//		case CID_VOLZP:
//			DeleteVolzp(plo);
//		break;
//
//		case CID_CNVO:
//			DeleteCnvo(plo);
//		break;
//
//		case CID_HBSK:
//			DeleteHbsk(plo);
//		break;
//
//		case CID_BOMB:
//			DeleteBomb(plo);
//		break;
//
//		case CID_MISSILE:
//			DeleteMissile(plo);
//		break;
//
//		case CID_ACCMISS:
//			DeleteAccmiss(plo);
//		break;
//
//		case CID_TARMISS:
//			DeleteTarmiss(plo);
//		break;
//
//		case CID_SPLMISS:
//			DeleteSplmiss(plo);
//		break;
//
//		case CID_GROUNDMISS:
//			DeleteGroundmiss(plo);
//		break;
//
//		case CID_FLY:
//			DeleteFly(plo);
//		break;
//
//		case CID_RAT:
//			DeleteRat(plo);
//		break;
//
//		case CID_ROH:
//			DeleteRoh(plo);
//		break;
//
//		case CID_ROC:
//			DeleteRoc(plo);
//		break;
//
//		case CID_ROST:
//			DeleteRost(plo);
//		break;
//
//		case CID_ROP:
//			DeleteRop(plo);
//		break;
//
//		case CID_DART:
//			DeleteDart(plo);
//		break;
//
//		case CID_UBV:
//			DeleteUbv(plo);
//		break;
//
//		case CID_UBP:
//			DeleteUbp(plo);
//		break;
//
//		case CID_JLO:
//			DeleteJlo(plo);
//		break;
//
//		case CID_PUFFT:
//			DeletePufft(plo);
//		break;
//
//		case CID_MRKV:
//			DeleteMrkv(plo);
//		break;
//
//		case CID_LGNB:
//			DeleteLgnb(plo);
//		break;
//
//		case CID_BLIPG:
//			DeleteBlipg(plo);
//		break;
//
//		case CID_CAMERA:
//			DeleteCamera(plo);
//		break;
//
//		case CID_LBONE:
//			DeleteLBone(plo);
//		break;
//
//		case CID_EMITTER:
//			DeleteEmitter(plo);
//		break;
//
//		case CID_LIGHT:
//			DeleteLight(plo);
//		break;
//
//		case CID_SCH:
//			DeleteAlo(plo);
//		break;
//
//		case CID_LIKH:
//			DeleteLikh(plo);
//		break;
//
//		case CID_CHKPNT:
//			DeleteChkpnt(plo);
//		break;
//
//		case CID_PROXY:
//			DeleteProxy(plo);
//		break;
//
//		case CID_SKY:
//			DeleteSky(plo);
//		break;
//
//		case CID_DPRIZE:
//			DeleteDprize(plo);
//		break;
//
//		case CID_CHARM:
//			DeleteCharm(plo);
//		break;
//
//		case CID_COIN:
//			DeleteCoin(plo);
//		break;
//
//		case CID_KEY:
//			DeleteKey(plo);
//		break;
//
//		case CID_GOLD:
//			DeleteGold(plo);
//		break;
//
//		case CID_LOCK:
//			DeleteLock(plo);
//		break;
//
//		case CID_LOCKG:
//			DeleteLockg(plo);
//		break;
//
//		case CID_TAIL:
//			DeleteTail(plo);
//		break;
//
//		case CID_ROB:
//			DeleteRob(plo);
//		break;
//
//		case CID_FLASH:
//			DeleteFlash(plo);
//		break;
//
//		case CID_DYSH:
//			DeleteDysh(plo);
//		break;
//
//		case CID_SCENTMAP:
//			DeleteScentmap(plo);
//		break;
//
//		case CID_WAYPOINT:
//			DeleteWaypoint(plo);
//		break;
//
//		case CID_TN:
//			DeleteTn(plo);
//		break;
//
//		case CID_JLOC:
//			DeleteJloc(plo);
//		break;
//
//		case CID_DIALOG:
//			DeleteDialog(plo);
//		break;
//
//		case CID_SPEAKER:
//			DeleteSpeaker(plo);
//		break;
//
//		case CID_ROPE:
//			DeleteRope(plo);
//		break;
//
//		case CID_WM:
//			DeleteWm(plo);
//		break;
//
//		case CID_PUFFB:
//			DeletePuffb(plo);
//		break;
//
//		case CID_CRBRAIN:
//			DeleteCrbrain(plo);
//		break;
//
//		case CID_MGC:
//			DeleteMgc(plo);
//		break;
//
//		case CID_JACKB:
//			DeleteJackb(plo);
//		break;
//
//		case CID_JACKN:
//			DeleteJackn(plo);
//		break;
//
//		case CID_JACKF:
//			DeleteJackf(plo);
//		break;
//
//		case CID_SW:
//			
//		break;
//
//		case CID_CM:
//			DeleteCm(plo);
//		break;
//
//		case CID_SHAPE:
//			DeleteShape(plo);
//		break;
//
//		case CID_HSHAPE:
//			DeleteHshape(plo);
//		break;
//
//		case CID_PIPE:
//			DeletePipe(plo);
//		break;
//
//		case CID_RAIL:
//			DeleteRail(plo);
//		break;
//
//		case CID_LANDING:
//			DeleteLanding(plo);
//		break;
//
//		case CID_XFM:
//			DeleteXfm(plo);
//		break;
//
//		case CID_WARP:
//			DeleteWarp(plo);
//		break;
//
//		case CID_TARGET:
//			DeleteTarget(plo);
//		break;
//
//		case CID_HND:
//			DeleteHnd(plo);
//		break;
//
//		case CID_EXPL:
//			DeleteExpl(plo);
//		break;
//
//		case CID_EXPLG:
//			DeleteExplg(plo);
//		break;
//
//		case CID_EXPLO:
//			DeleteExplo(plo);
//		break;
//
//		case CID_EXPLS:
//			DeleteExpls(plo);
//		break;
//
//		case CID_VOL:
//			DeleteVol(plo);
//		break;
//
//		case CID_RATHOLE:
//			DeleteRathole(plo);
//		break;
//
//		case CID_PUFFV:
//			DeletePuffv(plo);
//		break;
//
//		case CID_EXIT:
//			DeleteExit(plo);
//		break;
//
//		case CID_PNT:
//			DeletePnt(plo);
//		break;
//
//		case CID_PNTSV:
//			DeleteLo(plo);
//		break;
//
//		case CID_PNTS:
//			DeleteLo(plo);
//		break;
//
//		case CID_PNTVEC:
//			DeleteLo(plo);
//		break;
//
//		case CID_HPNT:
//			DeleteHpnt(plo);
//		break;
//
//		case CID_JMT:
//			DeleteJmt(plo);
//		break;
//
//		case CID_SPIRE:
//			DeleteSpire(plo);
//		break;
//
//		case CID_SCAN:
//			DeleteScan(plo);
//		break;
//
//		case CID_ASEG:
//			DeleteAseg(plo);
//		break;
//
//		case CID_ASEGBL:
//			DeleteAsegbl(plo);
//		break;
//
//		case CID_MAP:
//			DeleteMap(plo);
//		break;
//
//		case CID__VISZONE:
//			DeleteLo(plo);
//		break;
//
//		case CID_VISMAP:
//			DeleteVismap(plo);
//		break;
//
//		case CID_FRZG:
//			DeleteFrzg(plo);
//		break;
//
//		case CID_SM:
//			DeleteSm(plo);
//		break;
//
//		case CID_SGG:
//			DeleteSgg(plo);
//		break;
//
//		case CID_PATHZONE:
//			DeletePathzone(plo);
//		break;
//
//		case CID_RCHM:
//			DeleteRchm(plo);
//		break;
//
//		case CID_RWM:
//			DeleteRwm(plo);
//		break;
//
//		case CID_WR:
//			DeleteWr(plo);
//		break;
//
//		case CID_KEYHOLE:
//			DeleteKeyhole(plo);
//		break;
//
//		case CID_JSG:
//			DeleteJsg(plo);
//		break;
//
//		case CID_ASEGA:
//			DeleteAsega(plo);
//		break;
//
//		case CID_SMA:
//			DeleteSma(plo);
//		break;
//
//		default:
//			std::cout << "Default hit " << plo->pvtlo->cid<<"\n";
//		break;
//	}
//}
