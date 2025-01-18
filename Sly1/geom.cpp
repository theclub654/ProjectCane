#include "geom.h"

void InitGeom(GEOM* pgeom)
{
	pgeom->aedge.clear();
	pgeom->cpos = 0;
	pgeom->apos.clear();
	pgeom->csurf = 0;
	pgeom->asurf.clear();
	pgeom->cedge = 0;
}

void ReadGeom(GEOM *pgeom, CBinaryInputStream *pbis)
{
    pgeom->sRadius = pbis->F32Read();

    pgeom->cpos = pbis->U16Read();
    pgeom->apos.resize(pgeom->cpos);

    for (int i = 0; i < pgeom->cpos; i++)
        pgeom->apos[i] = pbis->ReadVector();

    pgeom->csurf = pbis->U16Read();

    pgeom->asurf.resize(pgeom->csurf);
    pgeom->mpisurfposCenter.resize(pgeom->csurf);
    pgeom->mpisurfsRadius.resize(pgeom->csurf);

    pgeom->cedge = pbis->U16Read();
    pgeom->aedge.resize(pgeom->cedge);

    for (int i = 0; i < pgeom->csurf; i++)
    {
        pbis->U16Read();
        pbis->U16Read();
        pbis->U16Read();
        pgeom->mpisurfsRadius[i] = pbis->F32Read();
        pgeom->mpisurfposCenter[i] = pbis->ReadVector();
        byte unk_0 = pbis->U8Read();

        for (int a = 0; a < unk_0; a++)
        {
            pgeom->indices.push_back(pbis->U16Read());
            pgeom->indices.push_back(pbis->U16Read());
            pbis->U16Read();
        }
    }

    if (pgeom->cpos != 0)
    {
        glGenVertexArrays(1, &pgeom->VAO);
        glBindVertexArray(pgeom->VAO);

        glGenBuffers(1, &pgeom->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, pgeom->VBO);
        glBufferData(GL_ARRAY_BUFFER, pgeom->apos.size() * sizeof(glm::vec3), pgeom->apos.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &pgeom->EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pgeom->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, pgeom->indices.size() * sizeof(uint16_t), pgeom->indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }
}