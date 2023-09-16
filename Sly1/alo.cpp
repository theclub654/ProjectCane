#include "alo.h"

void RemoveAloHierarchy(ALO *palo)
{
	DLI plo;

	// Loading objects child
	plo.m_pdl = &palo->dlChild;
	// Loading object's chile base offset to object child entry
	plo.m_ibDle = palo->dlChild.ibDle;

	plo.m_pdliNext = s_pdliFirst;

	// Storing parent object in global parent list
	s_pdliFirst = &plo;
	plo.m_ppv = (void**)plo.m_pdl;

	// Loading object header from RIPG object
	LO *LocalObject = (LO*)palo;

	while (true)
	{
		

		break;
	}
}

void InitAlo(ALO* palo)
{
	InitDl(&palo->dlChild, 0x1C + 0xC);
	InitDl(&palo->dlFreeze, 0x64 + 0xC);

	InitLo(palo);
	palo->sCelBorderMRD = 2139095039;
	palo->sMRD = 2139095039;
	palo->grfzon = -1;

	InitDl(&palo->dlAct, 8 + 0xC);
}

void AddAloHierarchy(ALO* palo)
{
	DLI plo;

	// Loading objects child object
	plo.m_pdl = &palo->dlChild;
	// Loading the object base offset to child entry list
	plo.m_ibDle = palo->dlChild.ibDle;
	// Loading the first parent of the object
	plo.m_pdliNext = s_pdliFirst;

	s_pdliFirst = &plo;

	while (true)
	{

	}
}

void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis)
{
	pbis->ReadMatrix();
	pbis->ReadVector();

	pbis->U8Read();
	pbis->U8Read();
	pbis->U8Read();
	pbis->U32Read();
	pbis->F32Read();
	pbis->F32Read();
	pbis->F32Read();
	pbis->F32Read();
	LoadOptionFromBrx(palo, pbis);
	byte unk = LoadGlobsetFromBrx(pbis);
	LoadAloAloxFromBrx(pbis);

	byte unk_8 = pbis->U8Read();

	for (int i = 0; i < unk_8; i++)
	{
		pbis->S16Read();
		if (unk < 0)
		{

		}
		else
		{
			for (int i = 0; i < unk; i++)
				pbis->F32Read();
		}
	}

	LoadSwObjectsFromBrx(0, palo, pbis);
}

void LoadAloAloxFromBrx(CBinaryInputStream* pbis)
{
	uint32_t unk_0 = pbis->U32Read();

	if (unk_0 != 0)
	{
		int unk_1;

		if (unk_0 & 1)
		{
			pbis->ReadMatrix();
		}

		if (unk_0 & 2)
		{
			pbis->ReadMatrix();
		}

		if (((unk_0 & 0xc) != 0) && (unk_1 = pbis->S16Read() != -1))
		{

		}

		if ((unk_0 & 0x10) != 0)
			unk_1 = pbis->S16Read();

		if ((unk_0 & 0x20) != 0)
		{
			unk_1 = pbis->S16Read();
			pbis->ReadVector(); // Read Vector
			pbis->ReadVector(); // Read Vector
			pbis->F32Read();
		}

		if ((unk_0 & 0x40) != 0)
		{
			unk_1 = pbis->S16Read();
			unk_1 = pbis->S16Read();
		}

		if ((unk_0 & 0x80) != 0)
		{
			pbis->ReadVector();
			pbis->F32Read();

			if ((unk_0 & 0x100) != 0)
			{
				pbis->S16Read();
				pbis->S16Read();
				pbis->S16Read();
				pbis->S16Read();
				pbis->ReadVector();
			}
		}

		if ((unk_0 & 0x200) != 0)
		{
			pbis->S16Read();
			pbis->S16Read();
		}

		if ((unk_0 & 0x400) != 0)
		{
			pbis->U8Read();
		}
	}
}