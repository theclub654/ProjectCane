#pragma once
#include "alo.h"

class DYSH : public ALO
{
	public:

	GLuint shadowTex;
	int shadowLayer;
	SHADOW* pshadowGen;
};

DYSH*NewDysh();
void InitDysh(DYSH* pdysh);
int  GetDyshSize();
void CloneDysh(DYSH* pdysh, DYSH* pdyshBase);
void SetDyshShadow(DYSH* pdysh, SHADOW* pshadow);
void RenderDyshSelf(DYSH* pdysh, CM* pcm, RO* pro);
void DeleteDysh(DYSH *pdysh);

extern glm::mat4 g_uvToClip;
