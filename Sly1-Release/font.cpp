#include "font.h"

void CTextBox::SetPos(float x, float y)
{
	this->m_y = y;
	this->m_x = x;
}

void CTextBox::SetSize(float dx, float dy)
{
	this->m_dy = dy;
	this->m_dx = dx;
}

void CTextBox::SetTextColor(glm::vec4* rgba)
{
	m_rgba = *rgba;
}

void CTextBox::SetHorizontalJust(JH jh)
{
	this->m_jh = jh;
}

void CTextBox::SetVerticalJust(JV jv)
{
	this->m_jv = jv;
}

void CTextEdge::SetPos(float x, float y)
{

}

void CTextEdge::SetSize(float dx, float dy)
{

}

void CTextEdge::SetTextColor(glm::vec4* rgba)
{

}

void CTextEdge::SetHorizontalJust(JH jh)
{

}

void CTextEdge::SetVerticalJust(JV jv)
{

}

CRichText::CRichText(char* achz, CFontBrx* pfont)
{
	m_achz = achz;
	m_pfontBase = pfont;
	m_pfontCur = pfont;

	// Default grayscale text color (RGBA = 128,128,128,255)
	glm::vec4 defaultColor(1.0f);

	m_rgbaBase = defaultColor;
	m_rgbaCur = defaultColor;
	m_rgbaSet = defaultColor;
	m_rgbaOther = defaultColor;

	Reset();
}

void CRichText::GetExtents(float* pdx, float* pdy, float dxMax)
{
	int nLines = ClineWrap((int)dxMax);
	float maxLineWidth = DxMaxLine();

	if (pdx) *pdx = maxLineWidth;
	if (pdy && m_pfontBase) {
		*pdy = static_cast<float>(nLines) * static_cast<float>(m_pfontBase->m_dyUnscaled) * m_pfontBase->m_ryScale;
	}
}

int CRichText::ClineWrap(float dx)
{
	Reset();  // Reset text state
	int numLines = 1;
	float currentLineWidth = 0.0f;

	while (true) {
		char ch = ChNext();  // Advance and get next character
		if (ch == '\0') {
			return numLines;  // End of text
		}

		if (ch == '\n') {
			currentLineWidth = 0.0f;
			++numLines;
			continue;
		}

		// Measure width of current character with current font
		float charWidth = m_pfontCur->DxFromCh(ch);
		currentLineWidth += charWidth;

		// Line wrapping
		if (dx > 0.0f && currentLineWidth > dx) {
			// Search backward for a word boundary (space/tab)
			char* wrapPos = m_pchCur - 1;
			while (wrapPos >= m_achz && *wrapPos != '\n') {
				if (*wrapPos == ' ' || *wrapPos == '\t') {
					*wrapPos = '\n';        // Insert line break
					m_pchCur = wrapPos + 1; // Update current char pointer
					currentLineWidth = 0.0f;
					++numLines;
					goto continue_loop;
				}
				--wrapPos;
			}

			// No space found; force wrap at current character
			*m_pchCur = '\n';
			++numLines;
			currentLineWidth = 0.0f;
		}

	continue_loop:
		continue;
	}
}

float CRichText::DxMaxLine()
{
	float maxLineWidth = 0.0f;
	float currentLineWidth = 0.0f;

	Reset();

	while (char ch = ChNext()) {
		if (ch == '\n') {
			// Commit current line width and reset
			maxLineWidth = std::max(maxLineWidth, currentLineWidth);
			currentLineWidth = 0.0f;
		}
		else if (m_pfontCur) {
			float charWidth = m_pfontCur->DxFromCh(ch);
			currentLineWidth += charWidth;
		}
	}

	// Final check in case the last line didn’t end with newline
	return std::max(maxLineWidth, currentLineWidth);
}

char CRichText::ChNext() 
{
	while (*m_pchCur != '\0') {
		char ch = *m_pchCur;

		// -------- & font / entity handling --------
		if (ch == '&') {
			char next = *(m_pchCur + 1);

			
			if (next == '&') {
				m_pchCur += 2;
				return '&';
			}

			
			if (next == '.') {
				m_pfontCur = m_pfontBase;
				m_rgbaCur = m_rgbaSet;   // PS2 restores to the *set* color
				m_pchCur += 2;
				continue;
			}
		
			int fontIndex = next - '0';
			if (fontIndex >= 1 && fontIndex < g_cfontBrx) {
				// copy font and scale relative to base
				g_afontBrx[fontIndex].CopyTo(&m_fontOther);

				float scaleBase = m_pfontBase->m_dyUnscaled * m_pfontBase->m_ryScale;
				float scaleOther = m_fontOther.m_dyUnscaled * m_fontOther.m_ryScale;
				float rx = (scaleOther != 0.0f) ? (scaleBase / scaleOther) : 1.0f;

				m_fontOther.PushScaling(rx, rx);
				m_pfontCur = &m_fontOther;

				// carry alpha; use "other" color like PS2 does
				float a = m_rgbaCur.a;
				m_rgbaCur = m_rgbaOther;
				m_rgbaCur.a = a;

				m_pchCur += 2;
				continue;
			}

			// Unknown '&' pattern:
			// DO NOT consume two chars. Return '&' and advance one,
			// so the following character is still parsed on the next call.
			++m_pchCur;
			return '&';
		}

		// -------- ~ color handling --------
		if (ch == '~') {
			char next = *(m_pchCur + 1);

			
			if (next == '~') {
				m_pchCur += 2;
				return '~';
			}

			
			if (next == '.') {
				m_rgbaSet = m_rgbaBase;
				m_rgbaCur = m_rgbaSet;
				m_pchCur += 2;
				continue;
			}

			
			if (std::isxdigit((unsigned char)next) &&
				std::isxdigit((unsigned char)*(m_pchCur + 2)) &&
				std::isxdigit((unsigned char)*(m_pchCur + 3)) &&
				std::isxdigit((unsigned char)*(m_pchCur + 4)) &&
				std::isxdigit((unsigned char)*(m_pchCur + 5)) &&
				std::isxdigit((unsigned char)*(m_pchCur + 6))) {

				unsigned int hexColor = std::strtoul(std::string(m_pchCur + 1, 6).c_str(), nullptr, 16);
				float a = m_rgbaCur.a; // keep alpha
				glm::vec4 color = {
					((hexColor >> 16) & 0xFF) / 255.0f * 0.5f,
					((hexColor >> 8) & 0xFF) / 255.0f * 0.5f,
					((hexColor) & 0xFF) / 255.0f * 0.5f,
					a
				};
				m_rgbaSet = m_rgbaCur = color;
				m_pchCur += 7; // '~' + 6 hex
				continue;
			}

			
			++m_pchCur;
			return '~';
		}

		// -------- Normal character --------
		++m_pchCur;
		return ch;
	}

	return '\0';
}

void CRichText::SetBaseColor(glm::vec4* rgba)
{
	m_rgbaBase = *rgba;
}

int CRichText::Cch()
{
	int count = 0;
	Reset();

	char ch;
	while ((ch = ChNext()) != '\0') {
		++count;
	}

	return count;
}

void CRichText::Trim(int cch)
{
	Reset();

	char ch = '\0';
	while (cch-- > 0) {
		ch = ChNext();
		if (ch == '\0') {
			break;
		}
	}

	// Null-terminate at current parse location
	*m_pchCur = '\0';
}

void CRichText::Reset()
{
	m_pchCur = m_achz;
	m_pfontCur = m_pfontBase;

	// Reset colors: start with base color, and apply it to current and set
	m_rgbaSet = m_rgbaBase;
	m_rgbaCur = m_rgbaBase;
}

void CRichText::Draw(CTextBox* ptbx)
{
	// ----- First pass: line widths for horizontal justification -----
	float lineWidths[64] = { 0.0f };
	int   lineCount = 0;

	Reset();
	for (;;)
	{
		char ch = ChNext();
		if (ch == '\0') break;
		if (ch == '\n')
		{
			if (lineCount < 63) ++lineCount;         // clamp to 64 lines
		}
		else
		{
			// Width must reflect current font as modified by ChNext.
			lineWidths[lineCount] += m_pfontCur->DxFromCh(ch);
		}
	}

	// ----- Vertical justification -----
	const float lineH = m_pfontBase->m_dyUnscaled * m_pfontBase->m_ryScale;
	const float totalH = (lineCount + 1) * lineH;
	float       yCursor = 0.0f;

	switch (ptbx->m_jv)
	{
	case JV_Top:    yCursor = ptbx->m_y;                               break;
	case JV_Bottom: yCursor = ptbx->m_y + ptbx->m_dy - totalH;         break;
	default:        yCursor = ptbx->m_y + (ptbx->m_dy - totalH) * 0.5f; break;
	}

	// ----- Base color (the per-run color still comes from ChNext) -----
	SetBaseColor(&ptbx->m_rgba);

	// ----- Second pass: draw -----
	Reset();

	int   currentLine = 0;
	bool  newLineStart = true;
	float xCursor = 0.0f;

	CFontBrx* prevFont = nullptr;  // “previously active” font object (PS2 tracked this)
	bool   didSetup = false;    // whether we called SetupDraw for the first font

	for (;;)
	{
		char ch = ChNext();
		if (ch == '\0') break;

		if (ch == '\n')
		{
			++currentLine;
			yCursor += lineH;
			newLineStart = true;
			continue;
		}

		// Compute X at the beginning of the line
		if (newLineStart)
		{
			switch (ptbx->m_jh)
			{
			case JH_Left:   xCursor = ptbx->m_x; break;
			case JH_Right:  xCursor = ptbx->m_x + ptbx->m_dx - lineWidths[currentLine]; break;
			default:        xCursor = ptbx->m_x + (ptbx->m_dx - lineWidths[currentLine]) * 0.5f;
				break;
			}
			newLineStart = false;
		}

		// PS2 logic: only act when the FONT POINTER changes (ChNext sets m_pfontCur)
		CFontBrx* curFont = m_pfontCur;

		if (!didSetup)
		{
			// First glyph: set up the font’s render state (PS2: SetupDraw)
			curFont->SetupDraw();
			
			didSetup = true;
			prevFont = curFont;
		}
		else if (curFont != prevFont)
		{
			// Font changed mid-stream (due to &n / &.)
			// PS2: SwitchTex0 — in GL, this should bind the new atlas & update per-font uniforms.
			//curFont->SwitchTex0(pgifs);
			glBindTexture(GL_TEXTURE_2D, curFont->m_pbmp->glDiffuseMap);
			prevFont = curFont;
		}

		// Draw glyph with the current font and color chosen by ChNext
		float dx = curFont->DxDrawCh(ch, xCursor, yCursor, m_rgbaCur);
		xCursor += dx;
	}

	if (didSetup)
		m_pfontCur->CleanUpDraw();
}

void CFontBrx::LoadFromBrx(CBinaryInputStream *pbis)
{
	FONTF fontf{};
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

	g_testFontBrx.push_back(this);

	m_pbmp = &g_abmp[fontf.ibmp];

	if (fontf.iclut == -1)
		m_pclut = nullptr;
	else
		m_pclut = &g_aclut[fontf.iclut];

	m_dxCharUnscaled = fontf.dxChar;
	m_dxSpaceUnscaled = fontf.dxSpace;
	m_dyUnscaled = fontf.dy;
	m_ryScale = fontf.rScale;
	m_rxScale = fontf.rScale;
	m_cglyff = fontf.cglyff;

	for (int i = 0; i < fontf.cglyff; i++)
	{
		GLYFF glyph;

		glyph.wch = pbis->U16Read();
		glyph.x   = pbis->U16Read();
		glyph.y   = pbis->U16Read();
		glyph.dx  = pbis->U16Read();

		m_aglyff[glyph.wch] = glyph;
	}
}

void CFontBrx::PostLoad()
{

}

GLYFF* CFontBrx::PglyffFromCh(char ch)
{
	auto it = m_aglyff.find(static_cast<uint8_t>(ch));

	if (it != m_aglyff.end())
		return &it->second;

	return nullptr;
}

float CFontBrx::DxFromPchz(char *pchz)
{
	if (pchz == nullptr) {
		return 0.0f;
	}

	float totalWidth = 0.0f;

	while (*pchz != '\0') {
		char ch = *pchz++;

		float charWidth = this->DxFromCh(ch);
		totalWidth += charWidth;
	}

	return totalWidth;
}

float CFontBrx::DxFromCh(char ch)
{
	// Get the glyph information for the character.
	GLYFF *pGlyph = PglyffFromCh(ch);

	// If the glyph is not found, return the space width (scaled).
	if (pGlyph == nullptr) {
		return static_cast<float>(m_dxSpaceUnscaled) * m_rxScale;
	}

	// If the glyph is found, return its width, scaled.
	return static_cast<float>(pGlyph->dx + 1) * m_rxScale;
}

CFontBrx* CFontBrx::PfontClone(float rx, float ry)
{
	CFontBrx* fontclone = new CFontBrx();

	fontclone->m_dxCharUnscaled = this->m_dxCharUnscaled;
	fontclone->m_dxSpaceUnscaled = this->m_dxSpaceUnscaled;
	fontclone->m_dyUnscaled = this->m_dyUnscaled;
	fontclone->m_rxScale = this->m_rxScale * rx;
	fontclone->m_ryScale = this->m_ryScale * ry;
	fontclone->m_csfr = this->m_csfr;
	std::memcpy(fontclone->m_asfr, this->m_asfr, sizeof(this->m_asfr));
	fontclone->m_fGstest = this->m_fGstest;
	fontclone->m_gstest = this->m_gstest;
	fontclone->m_z = this->m_z;

	fontclone->m_pbmp = this->m_pbmp;
	fontclone->m_pclut = this->m_pclut;
	fontclone->m_cglyff = this->m_cglyff;
	fontclone->m_aglyff = this->m_aglyff;
	fontclone->m_grffont = this->m_grffont;

	return fontclone;
}

void CFontBrx::FValid(char ch)
{

}

void CFontBrx::SetupDraw()
{
	glBlotShader.Use();

	glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));

	glBindTexture(GL_TEXTURE_2D, m_pbmp->glDiffuseMap);

	glBindVertexArray(g_gl.gao);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
}

float CFontBrx::DxDrawCh(char ch, float xChar, float yChar, glm::vec4 &rgba)
{
	GLYFF* glyph = PglyffFromCh(ch);

	if (!glyph)
		return static_cast<float>(m_dxSpaceUnscaled) * m_rxScale;

	float glyphW = (glyph->dx + 1) * m_rxScale;
	float glyphH = m_dyUnscaled * m_ryScale;


	float s0 = glyph->x / static_cast<float>(m_pbmp->bmpWidth);
	float t0 = (glyph->y + m_dyUnscaled) / static_cast<float>(m_pbmp->bmpHeight);
	float s1 = (glyph->x + glyph->dx) / static_cast<float>(m_pbmp->bmpWidth);
	float t1 = glyph->y / static_cast<float>(m_pbmp->bmpHeight);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(xChar, yChar + glyphH, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(glyphW, -glyphH, 1.0f));  // flips vertically

	glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform4f(uvRectLoc, s0, t0, s1, t1);
	glUniform4fv(blotColorLoc, 1, glm::value_ptr(rgba));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	return glyphW;
}

void CFontBrx::EdgeRect(CTextEdge* pte, CTextBox* ptbx)
{
	GLYFF *glyph = PglyffFromCh(pte->m_ch);

	if (!glyph) return;

	pte->m_pfont->PushScaling(pte->m_rxScaling, pte->m_ryScaling);

	float scaleX = m_rxScale;
	float scaleY = m_ryScale;

	float dxExtra = pte->m_dxExtra * scaleX;
	float dyExtra = pte->m_dyExtra * scaleY;
	float dyUnscaled = static_cast<float>(m_dyUnscaled);

	float glyphW = glyph->dx * scaleX;
	float glyphH = dyUnscaled * scaleY;

	float x = ptbx->m_x;
	float y = ptbx->m_y;
	float w = ptbx->m_dx;
	float h = ptbx->m_dy;

	float x0 = x - dxExtra;
	float y0 = y - dyExtra;
	float x1 = x + w + dxExtra;
	float y1 = y + h + dyExtra;

	float outerX0 = x0 - glyphW;
	float outerY0 = y0 - glyphH;
	float outerX1 = x1 + glyphW;
	float outerY1 = y1 + glyphH;

	float s0 = glyph->x / static_cast<float>(m_pbmp->bmpWidth);
	float t1 = glyph->y / static_cast<float>(m_pbmp->bmpHeight);
	float s1 = (glyph->x + glyph->dx) / static_cast<float>(m_pbmp->bmpWidth);
	float t0 = (glyph->y + dyUnscaled) / static_cast<float>(m_pbmp->bmpHeight);

	float sMid = (s0 + s1) * 0.5f;
	float tMid = (t0 + t1) * 0.5f;

	glm::vec4 color = pte->m_rgba;

	SetupDraw();

	// Draw 9-patch
	DrawPart(outerX0, outerY0, x0, y0, s0, t0, sMid, tMid, color); // Top-left
	DrawPart(x0, outerY0, x1, y0, sMid, t0, sMid, tMid, color);    // Top
	DrawPart(x1, outerY0, outerX1, y0, sMid, t0, s1, tMid, color); // Top-right

	DrawPart(outerX0, y0, x0, y1, s0, tMid, sMid, tMid, color); // Left
	DrawPart(x0, y0, x1, y1, sMid, tMid, sMid, tMid, color);	// Center
	DrawPart(x1, y0, outerX1, y1, sMid, tMid, s1, tMid, color); // Right

	DrawPart(outerX0, y1, x0, outerY1, s0, tMid, sMid, t1, color); // Bottom-left
	DrawPart(x0, y1, x1, outerY1, sMid, tMid, sMid, t1, color);    // Bottom
	DrawPart(x1, y1, outerX1, outerY1, sMid, tMid, s1, t1, color); // Bottom-right

	CleanUpDraw();

	pte->m_pfont->PopScaling();
}

void CFontBrx::PushScaling(float rx, float ry)
{
	// Pointer to the previous scale on the stack
	SFR* previousScale;

	// Determine the previous scale: use identity (1.0, 1.0) if stack is empty
	if (this->m_csfr < 1) {
		previousScale = &g_sfrOne;  // g_sfrOne is likely {1.0f, 1.0f}
	}
	else {
		previousScale = &this->m_asfr[this->m_csfr - 1];  // top of the stack
	}

	// Get index where the new scale will be stored
	int newStackIndex = this->m_csfr;

	// Increment stack counter
	this->m_csfr = newStackIndex + 1;

	// Store the new scale at the next available position
	SFR* newScale = &this->m_asfr[newStackIndex];
	newScale->rx = rx;
	newScale->ry = ry;

	// Update effective cumulative scale based on ratio to previous scale
	this->m_rxScale *= (rx / previousScale->rx);
	this->m_ryScale *= (ry / previousScale->ry);
}

void CFontBrx::CopyTo(CFontBrx* pfontDst)
{
	pfontDst->m_dxCharUnscaled = this->m_dxCharUnscaled;
	pfontDst->m_dxSpaceUnscaled = this->m_dxSpaceUnscaled;
	pfontDst->m_dyUnscaled = this->m_dyUnscaled;
	pfontDst->m_rxScale = this->m_rxScale;
	pfontDst->m_ryScale = this->m_ryScale;
	pfontDst->m_csfr = this->m_csfr;
	pfontDst->m_asfr[0] = m_asfr[0];
	pfontDst->m_asfr[1] = m_asfr[1];
	pfontDst->m_asfr[2] = m_asfr[2];
	pfontDst->m_asfr[3] = m_asfr[3];
	pfontDst->m_fGstest = this->m_fGstest;
	pfontDst->m_gstest = this->m_gstest;
	pfontDst->m_z = this->m_z;

	pfontDst->m_pbmp = this->m_pbmp;
	pfontDst->m_pclut = this->m_pclut;
	pfontDst->m_cglyff = this->m_cglyff;
	pfontDst->m_aglyff = this->m_aglyff;
	pfontDst->m_grffont = this->m_grffont;
}

void CFontBrx::GetExtents(char *pchz, float* pdx, float* pdy, float dxMax)
{
	// Wraps the string in-place and returns the number of lines after wrapping
	int numLines = ClineWrapPchz(pchz, dxMax);

	// Finds the width of the longest line in the (possibly modified) string
	float maxLineWidth = DxMaxLine(pchz);

	// Get scaled line height = (unscaled line height) * (vertical scale)
	float scaledLineHeight = static_cast<float>(this->m_dyUnscaled) * this->m_ryScale;

	// Return width if requested
	if (pdx != nullptr) {
		*pdx = maxLineWidth;
	}

	// Return total height if requested (numLines * scaledLineHeight)
	if (pdy != nullptr) {
		*pdy = static_cast<float>(numLines) * scaledLineHeight;
	}
}

void CFontBrx::PopScaling()
{
	int topIndex = this->m_csfr;
	this->m_csfr = topIndex - 1;

	// If there's only one or no scale on the stack, fall back to global identity scale
	SFR* prevScale = (topIndex - 1 < 1) ? &g_sfrOne : &this->m_asfr[topIndex - 2];

	// Adjust current scaling by dividing out the top scale and multiplying in the previous one
	this->m_rxScale *= (prevScale->rx / this->m_asfr[topIndex - 1].rx);
	this->m_ryScale *= (prevScale->ry / this->m_asfr[topIndex - 1].ry);
}

int CFontBrx::ClineWrapPchz(char *pchz, float dx)
{
	if (!pchz || dx < 0.0f) return 0;

	int lineCount = 1;
	float currentLineWidth = 0.0f;
	char* lineStart = pchz;
	char* cursor = pchz;

	while (*cursor != '\0') {
		if (*cursor == '\n') {
			currentLineWidth = 0.0f;
			++lineCount;
			lineStart = cursor + 1;
		}
		else {
			float charWidth = this->DxFromCh(*cursor);
			currentLineWidth += charWidth;

			if (dx > 0.0f && currentLineWidth > dx) {
				// Try to break at the last whitespace before this point
				char* breakAt = cursor;
				while (breakAt > lineStart && *breakAt != ' ' && *breakAt != '\t' && *breakAt != '\n') {
					--breakAt;
				}

				if (breakAt > lineStart && (*breakAt == ' ' || *breakAt == '\t')) {
					*breakAt = '\n';
					currentLineWidth = 0.0f;
					++lineCount;
					cursor = breakAt;  // resume after newline
					lineStart = cursor + 1;
				}
				else {
					// No whitespace found, hard break
					*cursor = '\n';
					++lineCount;
					currentLineWidth = 0.0f;
					lineStart = cursor + 1;
				}
			}
		}

		++cursor;
	}

	return lineCount;
}

float CFontBrx::DxMaxLine(char *pchz)
{
	float currentLineWidth = 0.0f;
	float maxLineWidth = 0.0f;

	while (*pchz != '\0') {
		if (*pchz == '\n') {
			if (currentLineWidth > maxLineWidth)
				maxLineWidth = currentLineWidth;
			currentLineWidth = 0.0f;
		}
		else {
			currentLineWidth += DxFromCh(*pchz);
		}
		++pchz;
	}

	// Final line (in case it doesn't end in '\n')
	if (currentLineWidth > maxLineWidth)
		maxLineWidth = currentLineWidth;

	return maxLineWidth;
}

float CFontBrx::DyWrapPchz(char* pchz, float dx)
{
	int numLines = ClineWrapPchz(pchz, dx);
	return static_cast<float>(m_dyUnscaled) * m_ryScale * static_cast<float>(numLines);
}

void CFontBrx::DrawPart(float x0, float y0, float x1, float y1,float s0, float t0, float s1, float t1, glm::vec4 &color)
{
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x0, y0, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(x1 - x0, y1 - y0, 1.0f));

	glm::vec4 uvRect = glm::vec4(s0, t0, s1, t1);

	glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform4fv(uvRectLoc, 1, glm::value_ptr(uvRect));
	glUniform4fv(blotColorLoc, 1, glm::value_ptr(color));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

void CFontBrx::DrawPchz(char* pchz, CTextBox* ptbx)
{
	if (!pchz || !ptbx) return;

	SetupDraw();

	float y = ptbx->m_y;

	if (ptbx->m_jv != JV_Top) {
		// Calculate total text height
		float totalHeight = m_dyUnscaled * m_ryScale;
		for (char* p = pchz; *p != '\0'; ++p) {
			if (*p == '\n') totalHeight += m_dyUnscaled * m_ryScale;
		}

		if (ptbx->m_jv == JV_Bottom) {
			y = ptbx->m_y + ptbx->m_dy - totalHeight;
		}
		else { // JV_Center
			y = ptbx->m_y + (ptbx->m_dy - totalHeight) * 0.5f;
		}
	}

	float x = ptbx->m_x;
	bool newLine = true;

	while (*pchz != '\0') {
		if (*pchz == '\n') {
			newLine = true;
			y += m_dyUnscaled * m_ryScale;
			++pchz;
			continue;
		}

		if (newLine) {
			if (ptbx->m_jh == JH_Left) {
				x = ptbx->m_x;
			}
			else {
				float lineWidth = 0.0f;
				for (char* p = pchz; *p != '\0' && *p != '\n'; ++p) {
					lineWidth += DxFromCh(*p);
				}

				if (ptbx->m_jh == JH_Right) {
					x = ptbx->m_x + ptbx->m_dx - lineWidth;
				}
				else { // JH_Center
					x = ptbx->m_x + (ptbx->m_dx - lineWidth) * 0.5f;
				}
			}
			newLine = false;
		}

		float charWidth = DxDrawCh(*pchz, x, y, ptbx->m_rgba);
		x += charWidth;
		pchz++;
	}

	CleanUpDraw();
}

void CFontBrx::CleanUpDraw()
{
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

void RenderGlyphQuad(float x, float y, float w, float h, float u0, float v0, float u1, float v1, const glm::vec4 &color)
{
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y + h, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(w, -h, 1.0f));  // flips vertically

	glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform4f(uvRectLoc, u0, v0, u1, v1);
	glUniform4fv(blotColorLoc, 1, glm::value_ptr(color));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

int g_cfontBrx;
std::vector <CFontBrx> g_afontBrx;

CFontBrx *g_pfont = nullptr;
CFontBrx *g_pfontScreenCounters = nullptr;
CFontBrx *g_pfontJoy = nullptr;

std::vector <CFontBrx*> g_testFontBrx;

SFR g_sfrOne = { 1.0, 1.0 };

GLuint u_projectionLoc = 0;
GLuint u_modelLoc = 0;
GLuint uvRectLoc = 0;
GLuint blotColorLoc = 0;
GLuint whiteTex = 0;