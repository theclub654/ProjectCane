#include "dysh.h"
#include "shadow.h"

namespace
{
    void ReleaseDyshTexture(DYSH* pdysh)
    {
        if (pdysh == nullptr)
            return;

        // The projected-shadow array is world-owned. DYSH only references one
        // of its layers and must never delete the shared texture.
        pdysh->shadowTex = 0;
        pdysh->shadowLayer = -1;

        if (pdysh->pshadowGen != nullptr)
        {
            pdysh->pshadowGen->glTexture = 0;

            if (pdysh->pshadowGen->pdysh == pdysh)
                pdysh->pshadowGen->pdysh = nullptr;
        }
    }
}

DYSH* NewDysh()
{
	return NewWorldObject<DYSH>();
}

void InitDysh(DYSH* pdysh)
{
	InitAlo(pdysh);
	pdysh->shadowTex = 0;
	pdysh->shadowLayer = -1;

	g_dynamicTextureCount++;
}

int GetDyshSize()
{
	return sizeof(DYSH);
}

void CloneDysh(DYSH *pdysh, DYSH *pdyshBase)
{
	CloneAlo(pdysh, pdyshBase);

	// The render texture is instance-owned.  Copying its OpenGL name would make
	// the source and clone delete the same texture and would leak it when the
	// clone receives its own texture in SetDyshShadow.
	pdysh->shadowTex = 0;
	pdysh->shadowLayer = -1;
	pdysh->pshadowGen = pdyshBase->pshadowGen;
}

void SetDyshShadow(DYSH *pdysh, SHADOW *pshadow)
{
	ReleaseDyshTexture(pdysh);

	pdysh->pshadowGen = pshadow;

    if (!pshadow)
        return;

    pshadow->pdysh = pdysh;
    pshadow->rsh.fDynamic = 1;

    // AllocateShadows assigns the shared array and layer after every shadow
    // object has been loaded.
    pshadow->glTexture = 0;
    pshadow->rsh.textureSlot = pshadow->textureSlot;
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
	ReleaseDyshTexture(pdysh);

	ReleaseWorldObject(pdysh);
}

glm::mat4 g_uvToClip =
{
    {-2.0, 0.0, 0.0, 0.0},
    {0.0, -2.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0},
    {1.0, 1.0, 0.0, -1.0}
};
