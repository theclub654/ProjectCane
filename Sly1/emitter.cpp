#include "emitter.h"

void LoadEmitMeshFromBrx(CBinaryInputStream* pbis)
{
	uint16_t cpos = pbis->U16Read();

	for (int i = 0; i < cpos; i++)
		pbis->ReadVector();

	uint16_t cemittri = pbis->U16Read();

	for (int i = 0; i < cemittri; i++)
	{
		for (int i = 0; i <= 2; i++)
			pbis->U16Read();

		pbis->F32Read();
	}

	pbis->ReadVector();
}

void LoadEmitblipColorsFromBrx(int crgba, CBinaryInputStream* pbis)
{
	byte colorRanges = pbis->U8Read();

	for (int i = 0; i < crgba; i++)
		pbis->U32Read();
}

void LoadEmitterFromBrx(CBinaryInputStream *pbis)
{
	LoadAloFromBrx(0, pbis);

	//std::cout << std::hex << pbis->file.tellg() << "\n";
	int8_t crvType = pbis->S8Read();

	if (crvType != -1)
	{
		switch (crvType)
		{
			case 0x0:
			LoadCrvlFromBrx(pbis);
			break;

			case 0x1:
			LoadCrvcFromBrx(pbis);
			break;
		}
	}

	if (loadEmitMesh == true)
	{
		loadEmitMesh = false;
		LoadEmitMeshFromBrx(pbis);
	}

	uint16_t crgba = pbis->U16Read();

	if (crgba != 0)
		LoadEmitblipColorsFromBrx(crgba, pbis);
}

void LoadExplgFromBrx(CBinaryInputStream *pbis)
{
	LoadXfmFromBrx(pbis);

	uint16_t numExplgObjs = pbis->S16Read();

	for (int i = 0; i < numExplgObjs; i++)
	{
		CID cid = (CID)pbis->S16Read();
		OID oid = (OID)pbis->S16Read();
		uint16_t isplice = pbis->S16Read();

		switch (cid)
		{
		case CID_ALO:
		{
			//ALO *palo = (ALO*)PloNew(cid, psw, paloParent, oid, isplice);
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_SO:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_MS:
		{
			//SO *pso = (SO*)PloNew(cid, psw, paloParent, oid, isplice);
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_PO:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_STEP:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_JT:
		{
			LoadJtFromBrx(0, pbis);
			break;
		}

		case CID_STEPGUARD:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_SMARTGUARD:
		{
			LoadStepGuardFromBrx(0, pbis);
			break;
		}

		case CID_GOMER:
		{
			LoadStepGuardFromBrx(0, pbis);
			break;
		}

		case CID_UBG:
		{
			LoadStepGuardFromBrx(0, pbis);
			break;
		}

		case CID_MBG:
		{
			LoadMbgFromBrx(0, pbis);
			break;
		}

		case CID_BHG:
		{
			LoadStepGuardFromBrx(0, pbis);
			break;
		}

		case CID_MURRAY:
		{
			LoadStepGuardFromBrx(0, pbis);
			break;
		}

		case CID_PUFFC:
		{
			LoadStepGuardFromBrx(0, pbis);
			break;
		}

		case CID_CRFOD:
		{
			LoadStepGuardFromBrx(0, pbis);
			break;
		}

		case CID_CRFODB:
		{
			LoadStepGuardFromBrx(0, pbis);
			break;
		}

		case CID_CRFODK:
		{
			LoadStepGuardFromBrx(0, pbis);
			break;
		}

		case CID_TANK:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_JP:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_HG:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_MECHA:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_ROV:
		{
			LoadRovFromBrx(0, pbis);
			break;
		}

		case CID_TURRET:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_VAULT:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_PUFFER:
		{
			LoadPufferFromBrx(0, pbis);
			break;
		}

		case CID_MGV:
		{
			LoadMgvFromBrx(0, pbis);
			break;
		}

		case CID_SUV:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_CYCLE:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_LGN:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_JACK:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_RIPG:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_WATER:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_BRK:
		{
			LoadBrkFromBrx(0, pbis);
			break;
		}

		case CID_BREAK:
		{
			LoadBrkFromBrx(0, pbis);
			break;
		}

		case CID_ALBRK:
		{
			LoadBrkFromBrx(0, pbis);
			break;
		}

		case CID_CAN:
		{
			LoadBrkFromBrx(0, pbis);
			break;
		}

		case CID_DARTGUN:
		{
			LoadBrkFromBrx(0, pbis);
			break;
		}

		case CID_SWP:
		{
			LoadBrkFromBrx(0, pbis);
			break;
		}

		case CID_FRAGILE:
		{
			LoadBrkFromBrx(0, pbis);
			break;
		}

		case CID_ZAPBREAK:
		{
			LoadBrkFromBrx(0, pbis);
			break;
		}

		case CID_BRKP:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_BUTTON:
		{
			LoadButtonFromBrx(0, pbis);
			break;
		}

		case CID_VOLBTN:
		{
			LoadVolbtnFromBrx(0, pbis);
			break;
		}

		case CID_JLOVOL:
		{
			LoadVolbtnFromBrx(0, pbis);
			break;
		}

		case CID_SQUISH:
		{
			LoadSquishFromBrx(0, pbis);
			break;
		}

		case CID_BONE:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_SPRIZE:
		{
			LoadSprizeFromBrx(0, pbis);
			break;
		}

		case CID_SCPRIZE:
		{
			LoadSprizeFromBrx(0, pbis);
			break;
		}

		case CID_LIFETKN:
		{
			LoadSprizeFromBrx(0, pbis);
			break;
		}

		case CID_CLUE:
		{
			LoadClueFromBrx(0, pbis);
			break;
		}

		case CID_ALARM:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_SENSOR:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_LASEN:
		{
			LoadLasenFromBrx(0, pbis);
			break;
		}

		case CID_CAMSEN:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_PRSEN:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_BARRIER:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_IKH:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_TZP:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_VOLZP:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_CNVO:
		{
			LoadCnvoFromBrx(0, pbis);
			break;
		}

		case CID_HBSK:
		{
			LoadHbskFromBrx(0, pbis);
			break;
		}

		case CID_BOMB:
		{
			LoadBombFromBrx(0, pbis);
			break;
		}

		case CID_MISSILE:
		{
			LoadMissileFromBrx(0, pbis);
			break;
		}


		case CID_ACCMISS:
		{
			LoadMissileFromBrx(0, pbis);
			break;
		}


		case CID_TARMISS:
		{
			LoadMissileFromBrx(0, pbis);
			break;
		}

		case CID_SPLMISS:
		{
			LoadMissileFromBrx(0, pbis);
			break;
		}

		case CID_GROUNDMISS:
		{
			LoadMissileFromBrx(0, pbis);
			break;
		}

		case CID_FLY:
		{
			LoadFlyFromBrx(0, pbis);
			break;
		}

		case CID_RAT:
		{
			LoadRatFromBrx(0, pbis);
			break;
		}

		case CID_ROH:
		{
			LoadRohFromBrx(0, pbis);
			break;
		}

		case CID_ROC:
		{
			LoadRocFromBrx(0, pbis);
			break;
		}

		case CID_ROST:
		{
			LoadRostFromBrx(0, pbis);
			break;
		}

		case CID_ROP:
		{
			LoadRopFromBrx(0, pbis);
			break;
		}

		case CID_DART:
		{
			LoadDartFromBrx(0, pbis);
			break;
		}

		case CID_UBV:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_UBP:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_DSP:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_JLO:
		{
			LoadJloFromBrx(0, pbis);
			break;
		}

		case CID_PUFFT:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_MRKV:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_LGNB:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		case CID_BLIPG:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_CAMERA:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_LBONE:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_EMITTER:
		{
			LoadEmitterFromBrx(pbis);
			break;
		}

		case CID_LIGHT:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_SCH:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_LIKH:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_CHKPNT:
		{
			LoadChkpntFromBrx(0, pbis);
			break;
		}

		case CID_PROXY:
		{
			//PROXY *pproxy = (PROXY*)PloNew(cid, psw, paloParent, oid, isplice);
			LoadProxyFromBrx(0, pbis);
			break;
		}

		case CID_SKY:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_DPRIZE:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_CHARM:
		{
			//DPRIZE *pdprize = (DPRIZE*)PloNew(cid, psw, paloParent, oid, isplice);
			LoadDprizeFromBrx(0, pbis);
			break;
		}

		case CID_COIN:
		{
			LoadDprizeFromBrx(0, pbis);
			break;
		}

		case CID_KEY:
		{
			LoadDprizeFromBrx(0, pbis);
			break;
		}

		case CID_GOLD:
		{
			LoadDprizeFromBrx(0, pbis);
			break;
		}

		case CID_LOCK:
		{
			LoadLockFromBrx(0, pbis);
			break;
		}

		case CID_LOCKG:
		{
			LoadLockgFromBrx(0, pbis);
			break;
		}

		case CID_TAIL:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_ROB:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_FLASH:
		{
			LoadFlashFromBrx(0, pbis);
			break;
		}

		case CID_DYSH:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_SCENTMAP:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_WAYPOINT:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_TN:
		{
			LoadTnFromBrx(0, pbis);
			break;
		}

		case CID_JLOC:
		{
			LoadJlocFromBrx(0, pbis);
			break;
		}

		case CID_DIALOG:
		{
			LoadDialogFromBrx(0, pbis);
			break;
		}

		case CID_SPEAKER:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_ROPE:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_WM:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_PUFFB:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_CRBRAIN:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_MGC:
		{
			LoadMgcFromBrx(0, pbis);
			break;
		}

		case CID_JACKB:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_JACKN:
		{
			LoadJacknFromBrx(0, pbis);
			break;
		}

		case CID_JACKF:
		{
			LoadAloFromBrx(0, pbis);
			break;
		}

		case CID_CM:
		{
			LoadLoFromBrx(0, pbis);
			break;
		}

		case CID_SHAPE:
		{
			LoadShapeFromBrx(0, pbis);
			break;
		}

		case CID_HSHAPE:
		{
			LoadShapeFromBrx(0, pbis);
			break;
		}

		case CID_PIPE:
		{
			LoadShapeFromBrx(0, pbis);
			break;
		}

		case CID_RAIL:
		{
			LoadShapeFromBrx(0, pbis);
			break;
		}

		case CID_LANDING:
		{
			LoadShapeFromBrx(0, pbis);
			break;
		}

		case CID_XFM:
		{
			LoadXfmFromBrx(pbis);
			break;
		}

		case CID_WARP:
		{
			LoadWarpFromBrx(0, pbis);
			break;
		}

		case CID_TARGET:
		{
			LoadXfmFromBrx(pbis);
			break;
		}

		case CID_HND:
		{
			LoadHndFromBrx(0, pbis);
			break;
		}

		case CID_EXPL:
		{
			LoadXfmFromBrx(pbis);
			break;
		}

		case CID_EXPLG:
		{
			LoadExplgFromBrx(pbis);
			break;
		}

		case CID_EXPLO:
		{
			LoadExploFromBrx(pbis);
			break;
		}

		case CID_EXPLS:
		{
			LoadExploFromBrx(pbis);
			break;
		}

		case CID_VOL:
		{
			LoadVolFromBrx(0, pbis);
			break;
		}

		case CID_RATHOLE:
		{
			LoadVolFromBrx(0, pbis);
			break;
		}

		case CID_PUFFV:
		{
			LoadVolFromBrx(0, pbis);
			break;
		}

		case CID_EXIT:
		{
			LoadExitFromBrx(0, pbis);
			break;
		}

		case CID_PNT:
		{
			//PNT *ppnt = (PNT*)PloNew(cid, psw, paloParent, oid, isplice);
			LoadPntFromBrx(0, pbis);
			break;
		}

		case CID_PNTSV:
		{
			LoadPntFromBrx(0, pbis);
			break;
		}

		case CID_PNTS:
		{
			LoadPntFromBrx(0, pbis);
			break;
		}

		case CID_PNTVEC:
		{
			LoadPntFromBrx(0, pbis);
			break;
		}

		case CID_HPNT:
		{
			LoadPntFromBrx(0, pbis);
			break;
		}

		case CID_JMT:
		{
			LoadPntFromBrx(0, pbis);
			break;
		}

		case CID_SPIRE:
		{
			LoadPntFromBrx(0, pbis);
			break;
		}

		case CID_SCAN:
		{
			LoadPntFromBrx(0, pbis);
			break;
		}

		case CID_ASEG:
		{
			LoadAsegFromBrx(0, pbis);
			break;
		}

		case CID_MAP:
		{
			LoadLoFromBrx(0, pbis);
			break;
		}

		case CID__VISZONE:
		{
			LoadLoFromBrx(0, pbis);
			break;
		}

		case CID_VISMAP:
		{
			LoadVismapFromBrx(pbis);
			break;
		}

		case CID_FRZG:
		{
			LoadLoFromBrx(0, pbis);
			break;
		}

		case CID_SM:
		{
			LoadSmFromBrx(0, pbis);
			break;
		}

		case CID_SGG:
		{
			LoadLoFromBrx(0, pbis);
			break;
		}

		case CID_PATHZONE:
		{
			LoadPathZoneFromBrx(0, pbis);
			break;
		}

		case CID_RCHM:
		{
			LoadRchmFromBrx(0, pbis);
			break;
		}

		case CID_RWM:
		{
			LoadLoFromBrx(0, pbis);
			break;
		}

		case CID_WR:
		{
			LoadLoFromBrx(0, pbis);
			break;
		}

		case CID_KEYHOLE:
		{
			//KEYHOLE *pkeyhole = (KEYHOLE*)PloNew(cid, psw, paloParent, oid, isplice);
			LoadKeyholeFromBrx(0, pbis);
			break;
		}

		case CID_JSG:
		{
			LoadLoFromBrx(0, pbis);
			break;
		}

		case CID_SMA:
		{
			LoadSoFromBrx(0, pbis);
			break;
		}

		default:
			std::cout << "Default case hit EXPLG:" << "\n";
			break;
		}
	}
}

void LoadExploFromBrx(CBinaryInputStream *pbis)
{
	LoadXfmFromBrx(pbis);
	int8_t crvType = pbis->S8Read();
	if (crvType != -1)
	{
		switch (crvType)
		{
			case 0x0:
			LoadCrvlFromBrx(pbis);
			break;

			case 0x1:
			LoadCrvcFromBrx(pbis);
			break;
		}
	}
	
	if (loadEmitMesh == true)
	{
		loadEmitMesh = false;
		LoadEmitMeshFromBrx(pbis);
	}

	uint16_t crgba = pbis->U16Read();

	if(crgba != 0)
		LoadEmitblipColorsFromBrx(crgba, pbis);
}