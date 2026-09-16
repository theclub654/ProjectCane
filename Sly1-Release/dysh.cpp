#include "dysh.h"
#include "shadow.h"

DYSH* NewDysh()
{
	return new DYSH{};
}

void InitDysh(DYSH* pdysh)
{
	InitAlo(pdysh);

	g_dynamicTextureCount++;
}

int GetDyshSize()
{
	return sizeof(DYSH);
}

void CloneDysh(DYSH *pdysh, DYSH *pdyshBase)
{
	CloneAlo(pdysh, pdyshBase);

    pdysh->shadowTex  = pdyshBase->shadowTex;
	pdysh->pshadowGen = pdyshBase->pshadowGen;
}

void SetDyshShadow(DYSH *pdysh, SHADOW *pshadow)
{
    pdysh->pshadowGen = pshadow;

    if (!pshadow)
        return;

    pshadow->pdysh = pdysh;
    pshadow->rsh.fDynamic = 1;

    const int w = g_gl.dyshWidth;
    const int h = g_gl.dyshHeight;

    glGenTextures(1, &pdysh->shadowTex);
    glBindTexture(GL_TEXTURE_2D, pdysh->shadowTex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    std::vector <uint8_t> clearData(w * h * 4, 0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, clearData.data());

    GLuint64 handle = glGetTextureHandleARB(pdysh->shadowTex);
    glMakeTextureHandleResidentARB(handle);

    pshadow->rsh.textureHandle[0] = uint32_t(handle & 0xFFFFFFFFull);
    pshadow->rsh.textureHandle[1] = uint32_t(handle >> 32);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderDyshSelf(DYSH* pdysh, CM* pcm, RO* pro)
{
    if (pdysh->pshadowGen != nullptr) 
    {
        RPL rpl{};

		if (pdysh->pvtalo != nullptr && pdysh->pvtalo->pfnUpdateAloInfluences != nullptr)
			pdysh->pvtalo->pfnUpdateAloInfluences(pdysh, pro);

        rpl.rp = RP_DynamicTexture;
        DupAloRo(pdysh, pro, &rpl.ro);

        rpl.pdysh = pdysh;
        SubmitRpl(&rpl);
    }
}

void DeleteDysh(DYSH *pdysh)
{
    if (pdysh->shadowTex)
    {
        GLuint64 handle = glGetTextureHandleARB(pdysh->shadowTex);

        glMakeTextureHandleNonResidentARB(handle);
        glDeleteTextures(1, &pdysh->shadowTex);
    }

	delete pdysh;
}

glm::mat4 g_uvToClip =
{
    {-2.0, 0.0, 0.0, 0.0},
    {0.0, -2.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0},
    {1.0, 1.0, 0.0, -1.0}
};
