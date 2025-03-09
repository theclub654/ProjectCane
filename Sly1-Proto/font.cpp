#include "font.h"

void CFontBrx::LoadFromBrx(CBinaryInputStream *pbis)
{
	FONTF fontf;

	// Loading texture ID for glyph from file
	fontf.ibmp = pbis->U16Read();
	// Loading CLUT ID for glyph from file
	fontf.iclut = pbis->U16Read();
	fontf.dxChar = pbis->U8Read();
	fontf.dxSpace = pbis->U8Read();
	fontf.dy = pbis->U8Read();
	fontf.bUnused = pbis->U8Read();
	fontf.rScale = pbis->F32Read();
	fontf.cglyff = pbis->U32Read();

	CFont.m_dxCharUnscaled = fontf.dxChar;
	CFont.m_dxSpaceUnscaled = fontf.dxSpace;
	CFont.m_dyUnscaled = fontf.dy;
	CFont.m_ryScale = fontf.rScale;
	CFont.m_rxScale = fontf.rScale;
	m_cglyff = fontf.cglyff;

	for (int i = 0; i < m_cglyff; i++)
	{
		GLYFF glyph;
		glyph.wch = pbis->U16Read();
		glyph.x = pbis->U16Read();
		glyph.y = pbis->U16Read();
		glyph.dx = pbis->U16Read();

		g_aglyff.push_back(glyph);
	}
}