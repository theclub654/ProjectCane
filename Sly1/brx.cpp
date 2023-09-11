#include "brx.h"

void StartupBrx()
{
	BuildEopids();
}

void* PloNew(CID cid, SW* parentSW, ALO* paloParent, OID oid, int isplice)
{
	switch (cid)
	{
		case CID_ALO:
		{
			ALO alo(cid, parentSW, paloParent, oid);
			ALO *palo = &alo;
			AppendDlEntry(PdlFromSwOid(palo->psw, oid), palo);
			InitAlo(palo);
			return palo;
		}

		case CID_MS:
		{
			SO *pso = (SO*)new ALO(cid, parentSW, paloParent, oid);
			AppendDlEntry(PdlFromSwOid(pso->psw, oid), pso);
			InitSo(pso);
			return pso;
		}

		case CID_JT:
		{
			JT *pjt = (JT*)new ALO(cid, parentSW, paloParent, oid);
			AppendDlEntry(PdlFromSwOid(pjt->psw, oid), pjt);
			InitJt(pjt);
			return pjt;
		}

		case CID_PROXY:
		{
			PROXY *pproxy = (PROXY*)new ALO(cid, parentSW, paloParent, oid);
			AppendDlEntry(PdlFromSwOid(pproxy->psw, oid), pproxy);
			InitProxy(pproxy);
			return pproxy;
		}

		case CID_CHARM:
		{
			DPRIZE* pdprize = (DPRIZE*)new ALO(cid, parentSW, paloParent, oid);
			AppendDlEntry(PdlFromSwOid(pdprize->psw, oid), pdprize);
			InitDprize(pdprize);
			return pdprize;
		}

		case CID_SW:
		{
			SW *psw = new SW(cid, paloParent, oid);
			psw->psw = psw;
			AppendDlEntry(PdlFromSwOid(psw->psw, oid), psw);
			InitSw(psw);
			return psw;
		}

		case CID_CM:
		{
			CM *pcm = new CM(cid, parentSW, paloParent, oid);
			AppendDlEntry(PdlFromSwOid(pcm->psw, oid), pcm);
			InitCm(pcm);
			return pcm;
		}

		case CID_PNT:
		{
			PNT pnt(cid, parentSW, paloParent, oid, isplice);
			PNT *ppnt = &pnt;
			AppendDlEntry(PdlFromSwOid(ppnt->psw, oid), ppnt);
			InitLo(ppnt);
			return ppnt;
		}

		case CID_KEYHOLE:
		{
			KEYHOLE *pkeyhole = new KEYHOLE(cid, parentSW, paloParent, oid);
			AppendDlEntry(PdlFromSwOid(pkeyhole->psw, oid), pkeyhole);
			InitKeyhole(pkeyhole);
			return pkeyhole;
		}
	}
}

void LoadSwObjectsFromBrx(SW *psw, ALO *paloParent, CBinaryInputStream *pbis)
{
	// Number of SW objects
	uint16_t numObjects = pbis->U16Read();

	for (int i = 0; i < numObjects; i++)
	{
		// Objects class ID
		CID cid = (CID)pbis->S16Read();
		// Objects ID
		OID oid = (OID)pbis->S16Read();
		/*std::cout << std::hex << cid << "\n";
		std::cout << std::hex << pbis->file.tellg() << "\n";*/
		// Objects splice event index
		int isplice = pbis->S16Read();

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
				//std::cout << std::hex << pbis->file.tellg() << "\n";
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
			{
				std::cout << "CID: " << std::hex <<(uint16_t)cid << "\n";
				std::cout << std::hex << pbis->file.tellg() << "\n";
				std::cout << "Default hit BRX:\n";
				break;
			}
		}
	}
}

DL* PdlFromSwOid(SW *psw, OID oid)
{
	return psw->objectParents + (oid * 0x95675 & 0x1ff);
}

void LoadOptionFromBrx(void* pvObject, CBinaryInputStream* pbis)
{
	// THIS FUNCTION WILL BE CHANGED LATER ON WHEN I GET AROUND TO RE
	uint16_t emitMeshFlag = 0;
	int16_t unk_id;

	while (true)
	{
	label0:
		unk_id = pbis->S16Read();

		if (unk_id == 0x2)
			goto label0;

		if (unk_id < 0)
			break;
		//std::cout << std::hex << pbis->file.tellg() <<"\n";
		uint32_t eopid = g_aeopid[(unk_id * 8)];
		uint32_t grfeopid = g_aeopid[(unk_id * 8) + 1];
		uint32_t crefReq = g_aeopid[(unk_id * 8) + 5];

		if ((grfeopid & 0x400) != 0)
		{
			for (int i = 0; i < crefReq; i++)
			{
				uint16_t unk_0 = pbis->S16Read();

				switch (unk_0)
				{
				case 0x0:
					pbis->U8Read();
					break;
				case 0x1:
					pbis->F32Read();
					break;
				case 0x2:
					pbis->F32Read();
					pbis->F32Read();
					pbis->F32Read();
					break;
				default:
					if (((int)unk_0 & 0x7fffffffU) >> 0xc == 1)
						pbis->S16Read();
					break;
				case 0x5:
					pbis->S32Read();
					break;
				case 0x6:
					pbis->S16Read();
					break;
				case 0x7:
					pbis->S16Read();
					break;
				case 0xE:
					pbis->S16Read();
					break;
				case 0xF:
					pbis->S16Read();
					break;
				case 0x10:
					pbis->S16Read();
					break;
				case 0x11:
					pbis->S16Read();
					break;
				case 0x14:
					pbis->S16Read();
					break;
				case 0x9:
					pbis->F32Read();
					pbis->F32Read();
					pbis->F32Read();
					break;
				case 0xA:
					pbis->F32Read();
					pbis->F32Read();
					break;
				}
			}

			goto label0;
		}

		if ((grfeopid & 0x80) != 0)
		{
			switch (eopid)
			{
			case 0x0:
				pbis->U8Read();
				break;
			case 0x1:
				pbis->F32Read();
				break;
			case 0x2:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0x9:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0xB:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0x3:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0xC:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0x4:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0x5:
				pbis->S32Read();
				break;
			default:
				emitMeshFlag = pbis->S16Read();
				if (emitMeshFlag == 0x4 && unk_id == 0x156)
					loadEmitMesh = true;
				else if (emitMeshFlag == 0x4 && unk_id == 0x18C)
					loadEmitMesh = true;
				break;
			case 0xA:
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0xD:
				pbis->U32Read();
				break;
			}
		}
		else
		{
			switch (eopid)
			{
			case 0x0:
				pbis->U8Read();
				break;
			case 0x1:
				pbis->F32Read();
				break;
			case 0x2:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0x9:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;

			case 0xB:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0x4:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0x5:
				pbis->S32Read();
				break;
			case 0x6:
				pbis->S16Read();
				break;
			case 0x7:
				pbis->S16Read();
				break;
			case 0xE:
				pbis->S16Read();
				break;
			case 0xF:
				pbis->S16Read();
				break;
			case 0x10:
				pbis->S16Read();
				break;
			case 0x11:
				pbis->S16Read();
				break;
			default:
				emitMeshFlag = pbis->S16Read();
				if (emitMeshFlag == 0x4 && unk_id == 0x1A5)
					loadEmitMesh = true;
				else if (emitMeshFlag == 0x4 && unk_id == 0x16E)
					loadEmitMesh = true;
				break;
			case 0xA:
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0xD:
				pbis->U32Read();
				break;
			}
		}

	}
}

void LoadOptionsFromBrx(void* pvObject, CBinaryInputStream* pbis)
{
	while (true)
	{
		// Reading eopid from binary file
		int16_t eopid = pbis->S16Read();

		if (eopid < 0) break;

		LoadOptionFromBrx(pvObject, pbis);
	}
}