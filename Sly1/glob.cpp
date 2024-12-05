#include "glob.h"

std::vector <SHD> g_ashd;
extern std::vector<ALO*> allSWAloObjs;
extern std::vector<LIGHT*> allSwLights;

void LoadGlobsetFromBrx(GLOBSET* pglobset ,CBinaryInputStream* pbis, ALO* palo)
{
    pglobset->cpsaa = 0;

    byte fRelight = pbis->U8Read();

    pglobset->cbnd = pbis->U8Read();
    pglobset->mpibndoid.resize(pglobset->cbnd);

    for (int i = 0; i < pglobset->cbnd; i++)
        pglobset->mpibndoid[i] = (OID)pbis->S16Read();

    pglobset->cpose = pbis->U8Read();
    pglobset->agPoses.resize(pglobset->cpose);

    for (int i = 0; i < pglobset->cpose; i++)
        pglobset->agPoses[i] = pbis->F32Read();

    // Loading number of submodels for model
    pglobset->cglob = pbis->U16Read();

    pglobset->aglob.resize(pglobset->cglob);
    pglobset->aglobi.resize(pglobset->cglob);

    // Loading each submodel for a model
    for (int i = 0; i < pglobset->cglob; i++)
    {
        uint16_t unk_5 = pbis->U16Read();
        
        if ((unk_5 & 1) != 0)
        {
            int instanceIndex = pbis->S16Read();
            glm::mat4 pdmat = pbis->ReadMatrix4();

            if (instanceIndex != 0)
                pglobset->aglob[instanceIndex].pdmat.push_back(pdmat);
        }

        else
        {
            pglobset->aglob[i].sMRD = 10000000000.000000;
            pglobset->aglob[i].sCelBorderMRD = 2000.0;
            pglobset->aglob[i].gZOrder = 0xFFFF7F7F;
            pglobset->aglob[i].uFog = 1.0;
            pglobset->aglob[i].rSubglobRadius = 1.0;
            pglobset->aglob[i].fDynamic = fRelight;
            pglobset->aglobi[i].uAlpha = 1.0;
        }

        if ((unk_5 & 2) != 0)
            pglobset->aglobi[i].grfzon = pbis->U32Read();

        if ((unk_5 & 0x200) != 0)
            pglobset->aglob[i].rSubglobRadius = pbis->F32Read();

        if ((unk_5 & 4) != 0)
            pbis->F32Read();
        
        if ((unk_5 & 8) != 0) // Skybox related
            pglobset->aglob[i].uFog = pbis->F32Read();

        if ((unk_5 & 0x10) != 0)
        {
            pglobset->aglob[i].sMRD = pbis->F32Read();

            if (pglobset->aglob[i].sMRD == 3.402823e+38)
                pglobset->aglob[i].sMRD = 10000000000.000000;
        }
        
        if ((unk_5 & 0x20) != 0)
            pbis->F32Read();

        if ((unk_5 & 0x40) != 0)
            PsaaLoadFromBrx(pbis);

        if ((unk_5 & 0x80) != 0)
        {
            GLEAM gleam;
            gleam.normal  = pbis->ReadVector();

            gleam.clqc.g0 = pbis->F32Read();
            gleam.clqc.g1 = pbis->F32Read();
            gleam.clqc.g2 = pbis->F32Read();
            gleam.clqc.g3 = pbis->F32Read();

            pglobset->aglob[i].pgleam.push_back(gleam);
        }

        if ((unk_5 & 0x100) != 0)
        {
            WRBG wrbg{};
            WRBG* pwrbg = &wrbg;

            pwrbg->oid = (OID)pbis->S16Read();
            pwrbg->weki.wek = (WEK)pbis->S8Read();

            if (pwrbg->weki.wek != -1)
            {
                pwrbg->weki.sInner = pbis->F32Read();
                pwrbg->weki.uInner = pbis->F32Read();
                pwrbg->weki.sOuter = pbis->F32Read();
                pwrbg->weki.uOuter = pbis->F32Read();
                pwrbg->weki.dmat   = pbis->ReadMatrix4();
            }

            pwrbg->cmat  = pbis->U8Read();
            pwrbg->fDpos = pbis->U8Read();
            pwrbg->fDuv  = pbis->U8Read();
            pglobset->aglob[i].pwrbg = pwrbg;
            pwrbg->pwrbgNextGlobset = pglobset->pwrbgFirst;
            pglobset->pwrbgFirst = pwrbg;
        }
         
        pglobset->aglob[i].posCenter = pbis->ReadVector();
        pglobset->aglob[i].sRadius   = pbis->F32Read();
        pglobset->aglob[i].oid       = (OID)pbis->S16Read();
        pglobset->aglob[i].rtck      = (RTCK)pbis->U8Read();
        pglobset->aglob[i].rp        = (RP)pbis->U8Read();
        pglobset->aglob[i].grfglob   = pbis->U8Read();
        
        BuildCmFgfn(g_pcm, pglobset->aglob[i].uFog, &pglobset->aglob[i].fgfn);

        if ((unk_5 & 1) == 0)
        {
            // Number of submodels
            // std::cout << "Model Start: " << std::hex << file.tellg()<<"\n";
            pglobset->aglob[i].csubglob = pbis->U16Read();
            pglobset->aglob[i].asubglob.resize(pglobset->aglob[i].csubglob);

            for (int a = 0; a < pglobset->aglob[i].csubglob; a++)
            {
                // Loading submodel origin
                pglobset->aglob[i].asubglob[a].posCenter = pbis->ReadVector();
                pglobset->aglob[i].asubglob[a].sRadius = pbis->F32Read();

                //std::cout << std::dec << "Vertex Count: " << (uint32_t)vertexCount << "\n";
                byte vertexCount = pbis->U8Read();
                //std::cout << std::dec << "Rotations Count: " << (uint32_t)rotationsCount << "\n";
                byte normalCount = pbis->U8Read();
                //std::cout << std::dec << "Vertex Color Count: " << (uint32_t)vertexColorCount << "\n";
                byte vertexColorCount = pbis->U8Read();
                //std::cout << std::dec << "Texcoords Count: " << (uint32_t)texCoordCount << "\n";
                byte texcoordCount = pbis->U8Read();
                //std::cout << std::dec << "Index Count: " << (uint32_t)indexCount << "\n";
                byte indexCount = pbis->U8Read();
                
                std::vector <glm::vec3> vertexes;
                vertexes.resize(vertexCount);

                std::vector <glm::vec3>normals;
                normals.resize(normalCount);

                std::vector <glm::vec4> vertexColors;
                vertexColors.resize(vertexColorCount);

                std::vector <glm::vec2> texcoords;
                texcoords.resize(texcoordCount);

                std::vector <VTXFLG> indexes;
                indexes.resize(indexCount);

                pbis->Align(4);

                //std::cout << "Vertices: " << std::hex << pbis->file.tellg() << "\n";
                for (int b = 0; b < vertexCount; b++)
                    vertexes[b] = pbis->ReadVector();

                //std::cout << "Normals: " << std::hex << pbis->file.tellg() << "\n";
                for (int c = 0; c < normalCount; c++)
                    normals[c] = pbis->ReadVector();
                
                //std::cout << "Vertex Colors: " << std::hex << pbis->file.tellg() << "\n";
                for (int d = 0; d < vertexColorCount; d++)
                {
                    vertexColors[d].r = (pbis->U8Read() * 2.0f) / 0x1FE;
                    vertexColors[d].g = (pbis->U8Read() * 2.0f) / 0x1FE;
                    vertexColors[d].b = (pbis->U8Read() * 2.0f) / 0x1FE;
                    vertexColors[d].a = (pbis->U8Read() * 2.0f) / 0x1FE;
                }

                //std::cout << "Texcoords: " << std::hex << pbis->file.tellg() << "\n";
                for (int e = 0; e < texcoordCount; e++)
                    texcoords[e] = pbis->ReadVector2();
                
                //std::cout << "Indexes: " << std::hex << pbis->file.tellg() << "\n\n";
                for (int f = 0; f < indexCount; f++)
                {
                    indexes[f].ipos    = pbis->U8Read();
                    indexes[f].inormal = pbis->U8Read();
                    indexes[f].iuv     = pbis->U8Read();
                    indexes[f].bMisc   = pbis->U8Read();
                }

                // Loading texture property
                pglobset->aglob[i].asubglob[a].pshd = &g_ashd[pbis->U16Read()];
                pglobset->aglob[i].asubglob[a].unSelfIllum = (pbis->U8Read() * 0x7FA6) / 0xFF;
                pglobset->aglob[i].asubglob[a].cibnd = pbis->U8Read();
                
                pbis->file.seekg(pglobset->aglob[i].asubglob[a].cibnd, SEEK_CUR);
                pbis->file.seekg(vertexCount * pglobset->aglob[i].asubglob[a].cibnd * 4, SEEK_CUR);
                
                if (pglobset->cpose != 0)
                {
                    uint16_t vertexCount1 = pbis->U16Read();

                    for (int i = 0; i < vertexCount1; i++)
                        pbis->ReadVector();

                    uint16_t normalCount = pbis->U16Read();

                    for (int i = 0; i < normalCount; i++)
                        pbis->ReadVector();

                    for (int i = 0; i < pglobset->cpose; i++)
                    {
                        for (int i = 0; i < indexCount; i++)
                        {
                            pbis->U8Read();
                            pbis->U8Read();
                        }

                        for (int i = 0; i < indexCount; i++)
                        {
                            pbis->U8Read();
                            pbis->U8Read();
                        }
                    }
                }

                if (pglobset->aglob[i].asubglob[a].pshd->shdk == SHDK_ThreeWay)
                    pglobset->aglob[i].asubglob[a].fThreeWay = 1;

                BuildSubGlob(&pglobset->aglob[i].asubglob[a], pglobset->aglob[i].asubglob[a].pshd, vertexes, normals, vertexColors, texcoords, indexes);
            }

            uint16_t numSubMesh1 = pbis->U16Read();

            for (int i = 0; i < numSubMesh1; i++)
            {
                byte vertexCount2 = pbis->U8Read();

                for (int i = 0; i < vertexCount2; i++)
                    pbis->ReadVector();

                byte indexCount = pbis->U8Read();

                for (int i = 0; i < indexCount; i++)
                {
                    pbis->U8Read();
                    pbis->U8Read();
                    pbis->U8Read();
                    pbis->U8Read();
                }

                byte unk_26 = pbis->U8Read();

                for (int i = 0; i < unk_26; i++)
                    pbis->U8Read();

                pbis->file.seekg(unk_26 * vertexCount2 * 4, SEEK_CUR);

                if (pglobset->cpose != 0)
                {
                    uint16_t vertexCount3 = pbis->U16Read();

                    for (int i = 0; i < vertexCount3; i++)
                        pbis->ReadVector();

                    for (int i = 0; i < pglobset->cpose; i++)
                    {
                        for (int i = 0; i < vertexCount2; i++)
                        {
                            pbis->U16Read();
                        }
                    }
                }
            }
        }
    }
}

void BuildSubGlob(SUBGLOB* subglob, SHD *pshd, std::vector <glm::vec3> positions, std::vector <glm::vec3> normals, std::vector <glm::vec4> colors, std::vector <glm::vec2> texcoords, std::vector <VTXFLG> indexes)
{
    for (int i = 0; i < indexes.size(); i++)
    {
        VERTICE vertice;

        vertice.pos = positions[indexes[i].ipos];

        if (indexes[i].inormal == 0xFF)
            vertice.normal = glm::vec3(0.0);
        else
            vertice.normal = normals[indexes[i].inormal];

        if ((indexes[i].bMisc & 0x7F) == 0x7F)
            vertice.color = pshd->rgba;
        else
            vertice.color = pshd->rgba * colors[indexes[i].bMisc & 0x7F];

        if (indexes[i].iuv == 0xFF)
            vertice.uv = glm::vec2{0.0};
        else
            vertice.uv = texcoords[indexes[i].iuv];

        subglob->vertices.push_back(vertice);
    }
    
    uint32_t idx = 0;
    for (int i = 2; i < indexes.size(); i++)
    {
        if (!(indexes[idx + 2].bMisc & 0x80))
        {
            subglob->indices.push_back(idx + 0);
            subglob->indices.push_back(idx + 1);
            subglob->indices.push_back(idx + 2);
        }

        idx++;
    }

    glGenVertexArrays(1, &subglob->VAO);
    glBindVertexArray(subglob->VAO);

    glGenBuffers(1, &subglob->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, subglob->VBO);
    glBufferData(GL_ARRAY_BUFFER, subglob->vertices.size() * sizeof(VERTICE), subglob->vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &subglob->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subglob->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, subglob->indices.size() * sizeof(uint16_t), subglob->indices.data(), GL_STATIC_DRAW);

    // Vertex Position's
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)offsetof(VERTICE, pos));
    glEnableVertexAttribArray(0);

    // Normal's
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)offsetof(VERTICE, normal));
    glEnableVertexAttribArray(1);

    // Vertex Color's
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)offsetof(VERTICE, color));
    glEnableVertexAttribArray(2);

    // Uv's
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)offsetof(VERTICE, uv));
    glEnableVertexAttribArray(3);
}