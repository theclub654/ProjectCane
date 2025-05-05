#include "zap.h"

TZP* NewTzp()
{
	return new TZP{};
}

void InitTzp(TZP* ptzp)
{
	InitSo(ptzp);
}

int GetTzpSize()
{
	return sizeof(TZP);
}

void CloneTzp(TZP* ptzp, TZP* ptzpBase)
{
	CloneSo(ptzp, ptzpBase);

	// Clone array of RZPR elements
	ptzp->crzpr = ptzpBase->crzpr;
	for (int i = 0; i < ptzp->crzpr; ++i)
	{
		ptzp->arzpr[i] = ptzpBase->arzpr[i];
	}

	ptzp->zpd.pso = ptzp;
}

void RenderTzpAll(TZP* ptzp, CM* pcm, RO* pro)
{
	RenderAloAll(ptzp, pcm, pro);
}

void DrawZap(RPL* prpl)
{
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "rko"), 3);
	glUniformMatrix4fv(glGetUniformLocation(glGlobShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(prpl->ro.model));

	glBindVertexArray(*prpl->ro.VAO);
	glDrawElements(GL_LINES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);
}

void DeleteTzp(TZP *ptzp)
{
	delete ptzp;
}

VOLZP* NewVolzp()
{
	return new VOLZP{};
}

void InitVolzp(VOLZP* pvolzp)
{
	InitTzp(pvolzp);
}

int GetVolzpSize()
{
	return sizeof(VOLZP);
}

void CloneVolzp(VOLZP* pvolzp, VOLZP* pvolzpBase)
{
	CloneTzp(pvolzp, pvolzpBase);
}

void DeleteVolzp(VOLZP *pvolzp)
{
	delete pvolzp;
}
