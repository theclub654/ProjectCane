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
    
	m_aglyff.clear();

	for (uint32_t i = 0; i < m_cglyff; i++)
	{
		GLYFF glyff;

		glyff.wch = pbis->U16Read();
		glyff.x = pbis->U16Read();
		glyff.y = pbis->U16Read();
		glyff.dx = pbis->U16Read();

		m_aglyff[glyff.wch] = glyff;
	}
}

void CFontBrx::PostLoad()
{

}

GLYFF* CFontBrx::PglyffFromCh(char ch)
{
	uint16_t wch = static_cast<uint8_t>(ch);

	auto it = m_aglyff.find(wch);

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

		// Call virtual function: this->vtable->DxFromCh(this + offset, ch)
		// The original code offsets `this` to access `m_dxCharUnscaled` correctly.
		// It effectively uses: this->DxFromCh(ch)
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

void CFontBrx::DxDrawCh(char* text, float x, float y, glm::vec4 rgba)
{
	glTextShader.Use();

	glDisable(GL_DEPTH_TEST);

	glm::mat4 projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f);
	GLint projLoc = glGetUniformLocation(glTextShader.ID, "u_projection");

	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	float cursorX = x;

	glBindTexture(GL_TEXTURE_2D, m_pbmp->glDiffuseMap);

	glUniform4fv(glGetUniformLocation(glTextShader.ID, "textColor"), 1, glm::value_ptr(rgba));

	while (*text)
	{
		GLYFF* glyph = PglyffFromCh(*text);

		if (glyph != nullptr)
		{
			float u0 = glyph->x / (float)m_pbmp->bmpWidth;
			float v0 = 1.0f - (glyph->y + m_dyUnscaled) / (float)m_pbmp->bmpHeight;

			float u1 = (glyph->x + glyph->dx) / (float)m_pbmp->bmpWidth;
			float v1 = 1.0f - glyph->y / (float)m_pbmp->bmpHeight;

			float w = (glyph->dx + 1) * m_rxScale;
			float h = m_dyUnscaled * m_ryScale;

			RenderGlyphQuad(cursorX, y, w, h, u0, v0, u1, v1);
			cursorX += w;
		}
		else
		{
			cursorX += m_dxSpaceUnscaled * m_rxScale;
		}
		++text;
	}

	glDisable(GL_BLEND);
}

CFont* CFontBrx::PFontClone(float rx, float ry)
{
	return nullptr;
}

void CFontBrx::FValid(char ch)
{

}

void CFontBrx::SetupDraw(CTextBox* ptbxClip)
{

}

void CFontBrx::DxDrawCh(char ch, float xChar, float yChar, u32 rgba)
{

}

void CFontBrx::EdgeRect(CTextEdge* pte, CTextBox* ptbx)
{
	GLYFF* glyph = PglyffFromCh(pte->m_ch);

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

	// Outer boundaries for edge rendering
	float x0 = x - dxExtra;
	float y0 = y - dyExtra;
	float x1 = x + w + dxExtra;
	float y1 = y + h + dyExtra;

	float outerX0 = x0 - glyphW;
	float outerY0 = y0 - glyphH;
	float outerX1 = x1 + glyphW;
	float outerY1 = y1 + glyphH;

	// Texture coordinates
	float s0 = glyph->x / static_cast<float>(m_pbmp->bmpWidth);
	float t0 =  (glyph->y + dyUnscaled) / m_pbmp->bmpHeight;
	float s1 = (glyph->x + glyph->dx) / static_cast<float>(m_pbmp->bmpWidth);
	float t1 =  (glyph->y) / m_pbmp->bmpHeight;

	glm::vec4 color = pte->m_rgba;

	glTextShader.Use();

	glUniformMatrix4fv(glGetUniformLocation(glTextShader.ID, "u_projection"), 1, GL_FALSE, glm::value_ptr(g_gl.textProjection));

	// Draw corners
	DrawPart(outerX0, outerY0, x0, y0, s0, t0, s1, t1, color);       // top-left
	DrawPart(x1, outerY0, outerX1, y0, s0, t0, s1, t1, color);       // top-right
	DrawPart(outerX0, y1, x0, outerY1, s0, t0, s1, t1, color);       // bottom-left
	DrawPart(x1, y1, outerX1, outerY1, s0, t0, s1, t1, color);       // bottom-right
	
	// Draw sides
	DrawPart(outerX0, y0, x0, y1, s0, t0, s1, t1, color);            // left
	DrawPart(x1, y0, outerX1, y1, s0, t0, s1, t1, color);            // right
	DrawPart(x0, outerY0, x1, y0, s0, t0, s1, t1, color);            // top
	DrawPart(x0, y1, x1, outerY1, s0, t0, s1, t1, color);            // bottom

	// Optionally draw center fill (if desired)
	DrawPart(x0, y0, x1, y1, s0, t0, s1, t1, color);              // center

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
	if (!pchz) return 0;

	int lineCount = 1;
	float lineWidth = 0.0f;
	char* cursor = pchz;

	while (*cursor != '\0') {
		char ch = *cursor;

		if (ch == '\n') {
			// Explicit newline: reset line width and count a new line
			lineWidth = 0.0f;
			++lineCount;
		}
		else {
			// Get character width
			float charWidth = this->DxFromCh(ch);
			lineWidth += charWidth;

			// Check for line wrapping
			if (dx > 0.0f && lineWidth > dx) {
				// Search backward for a breakable character
				char* breakPoint = cursor;
				while (breakPoint > pchz && *breakPoint != '\n') {
					if (*breakPoint == ' ' || *breakPoint == '\t') {
						*breakPoint = '\n';
						lineWidth = 0.0f;
						++lineCount;
						cursor = breakPoint;
						goto continue_loop;
					}
					--breakPoint;
				}

				// If no space or tab found, force a break at current position
				*cursor = '\n';
				lineWidth = 0.0f;
				++lineCount;
			}
		}

	continue_loop:
		++cursor;
	}

	return lineCount;
}

float CFontBrx::DxMaxLine(char *pchz)
{
	if (!pchz) return 0.0f;

	float maxWidth = 0.0f;
	float currentLineWidth = 0.0f;

	while (*pchz != '\0') {
		if (*pchz == '\n') {
			// Commit current line width to maxWidth if larger
			if (currentLineWidth > maxWidth) {
				maxWidth = currentLineWidth;
			}
			currentLineWidth = 0.0f;
		}
		else {
			// Add character width to current line
			float charWidth = this->DxFromCh(*pchz);
			currentLineWidth += charWidth;
		}
		++pchz;
	}

	// Final check in case the last line had no newline
	if (currentLineWidth > maxWidth) {
		maxWidth = currentLineWidth;
	}

	return maxWidth;
}

float CFontBrx::DyWrapPchz(char* pchz, float dx)
{
	int numLines = ClineWrapPchz(pchz, dx);
	return static_cast<float>(m_dyUnscaled) * m_ryScale * static_cast<float>(numLines);
}

void CFontBrx::DrawPart(float x0, float y0, float x1, float y1,float s0, float t0, float s1, float t1,glm::vec4 color)
{
	// Compute model matrix
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x0, y0, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(x1 - x0, y1 - y0, 1.0f));

	// Compute UV rectangle
	glm::vec4 uvRect = glm::vec4(s0, t0, s1, t1);

	// Upload uniforms
	glUniformMatrix4fv(glGetUniformLocation(glTextShader.ID, "u_model"), 1, GL_FALSE, glm::value_ptr(model));

	glUniform4fv(glGetUniformLocation(glTextShader.ID, "u_uvRect"),  1, glm::value_ptr(uvRect));
	glUniform4fv(glGetUniformLocation(glTextShader.ID, "textColor"), 1, glm::value_ptr(color));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_DEPTH_TEST);

	// Bind VAO and draw
	glBindVertexArray(g_gl.gao);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0); // assuming you're using an EBO

	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

void CFontBrx::DrawPchz(char* pchz, CTextBox* ptbx)
{

}

CRichText::CRichText(char *achz, CFont* pfont)
{
	// Set default RGBA values to medium gray with full alpha
	glm::vec4 defaultColor = { 0x80, 0x80, 0x80, 0xFF };

	m_rgbaCur = defaultColor;
	m_rgbaOther = defaultColor;
	m_rgbaBase = defaultColor;
	m_rgbaSet = defaultColor;

	// Initialize virtual table for m_fontOther if needed (likely legacy)
	//m_fontOther.__vtable = reinterpret_cast<CFontBrx__vtable*>(CFontBrx_virtual_table);

	// Reset internal state
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

	Reset();  // This sets up m_pchCur and other state

	char ch;
	while ((ch = ChNext()) != '\0') {
		if (ch == '\n') {
			if (currentLineWidth > maxLineWidth) {
				maxLineWidth = currentLineWidth;
			}
			currentLineWidth = 0.0f;
		}
		else {
			// Use the currently active font set by ChNext()
			float charWidth = m_pfontCur->DxFromCh(ch);
			currentLineWidth += charWidth;
		}
	}

	// Final line
	if (currentLineWidth > maxLineWidth) {
		maxLineWidth = currentLineWidth;
	}

	return maxLineWidth;
}

char CRichText::ChNext() {
	bool bVar9 = false;
	char* pcVar5 = m_pchCur;

	while (*pcVar5 != '\0') {
		char c = *pcVar5;

		if (c == '&') {
			char nextChar = *(pcVar5 + 1);

			if (nextChar == '&') {
				bVar9 = true;
				m_pchCur = pcVar5 + 1;  // Skip over the '&&'
				return bVar9;  // Exit early
			}
			else {
				int index = nextChar - '0';  // Assume nextChar is a digit

				if (nextChar == '.') {
					m_pfontCur = m_pfontBase;
					m_rgbaCur = m_rgbaSet;
					m_pchCur = pcVar5 + 2;  // Skip over the "&."
				}
				else if (index < 1 || index >= g_cfontBrx) {
					m_pfontCur = m_pfontBase;
					m_rgbaCur = m_rgbaBase;
					bVar9 = true;
				}
				else {
					g_afontBrx[index].CopyTo(&m_fontOther);
					//CFontBrx::CopyTo(g_afontBrx + index, &m_fontOther);
					m_pfontCur = &m_fontOther;
					float rx = static_cast<float>(m_pfontBase->m_dyUnscaled) * m_pfontBase->m_ryScale /
						static_cast<float>(m_fontOther.m_dyUnscaled * m_fontOther.m_ryScale);
					m_fontOther.PushScaling(rx, rx);
					//CFont::PushScaling(&m_fontOther, rx, rx);
					m_rgbaCur = m_rgbaOther;
					m_pchCur = pcVar5 + 2;  // Skip over the "&X"
				}
			}
		}
		else if (c == '~') {
			char nextChar = *(pcVar5 + 1);

			if (nextChar == '~') {
				bVar9 = true;
				m_pchCur = pcVar5 + 1;  // Skip over the '~~'
				return bVar9;  // Exit early
			}
			else {
				// Handle the tilde color assignment
				if (nextChar == '.') {
					m_rgbaCur = m_rgbaSet;
					m_pchCur = pcVar5 + 2;  // Skip over the "~."
				}
				else {
					m_rgbaCur = m_rgbaBase;
					m_pchCur = pcVar5 + 2;  // Skip over the "~X"
				}
			}
		}

		++pcVar5;
	}

	return bVar9;  // Return true if we did any modification
}

void CRichText::Reset()
{
	m_pchCur = m_achz;
	m_pfontCur = m_pfontBase;

	// Reset colors: start with base color, and apply it to current and set
	m_rgbaSet = m_rgbaBase;
	m_rgbaCur = m_rgbaBase;
}

void RenderGlyphQuad(float x, float y, float w, float h, float u0, float v0, float u1, float v1)
{
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y + h, 0.0f));
	model = glm::scale(model, glm::vec3(w, -h, 1.0f)); // Negative scale flips vertically

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUniformMatrix4fv(glGetUniformLocation(glTextShader.ID, "u_model"), 1, GL_FALSE, glm::value_ptr(model));

	GLint uvRectLoc = glGetUniformLocation(glTextShader.ID, "u_uvRect");
	glUniform4f(uvRectLoc, u0, v0, u1, v1);

	glBindVertexArray(g_gl.gao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

int g_cfontBrx;
std::vector <CFontBrx> g_afontBrx;

CFontBrx* g_pfont = nullptr;
CFontBrx* g_pfontScreenCounters = nullptr;
CFontBrx* g_pfontJoy = nullptr;

CTextEdge g_teAttract;
CTextEdge g_teWmc;
CTextEdge g_teLogo;
CTextEdge g_teNote;
CTextEdge g_tePrompt;

SFR g_sfrOne = { 1.0, 1.0 };