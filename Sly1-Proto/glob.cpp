#include "glob.h"

std::vector <SHD> g_ashd;
extern std::vector<ALO*> allSWAloObjs;
extern std::vector<LIGHT*> allSwLights;

void LoadGlobsetFromBrx(GLOBSET *pglobset, short cid ,ALO *palo, CBinaryInputStream *pbis)
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

    int fCloneSubGlob = 0;
    int fCelBorder = 1;

    // Loading each submodel for a model
    for (int i = 0; i < pglobset->cglob; i++)
    {
        uint16_t unk_5 = pbis->U16Read();
        
        if ((unk_5 & 1) == 0)
        {
            fCloneSubGlob = 0;

            pglobset->aglob[i].sMRD = 10000000000.000000;
            pglobset->aglob[i].sCelBorderMRD = 2000.0;
            pglobset->aglob[i].gZOrder = 0xFFFF7F7F;
            pglobset->aglob[i].uFog = 1.0;
            pglobset->aglob[i].rSubglobRadius = 1.0;
            pglobset->aglob[i].fDynamic = fRelight;
            pglobset->aglobi[i].uAlpha = 1.0;
        }

        else
        {
            fCloneSubGlob = unk_5 & 1;

            int instanceIndex = pbis->S16Read();
            pglobset->aglob[i].instanceIndex = instanceIndex;
            pglobset->aglob[i].posCenter = pglobset->aglob[instanceIndex].posCenter;
            pglobset->aglob[i].sRadius = pglobset->aglob[instanceIndex].sRadius;
            pglobset->aglob[i].rp = pglobset->aglob[instanceIndex].rp;
            pglobset->aglob[i].fThreeWay = pglobset->aglob[instanceIndex].fThreeWay;
            pglobset->aglob[i].sMRD = pglobset->aglob[instanceIndex].sMRD;
            pglobset->aglobi[i] = pglobset->aglobi[instanceIndex];

            glm::mat4 instanceModelMatrix = pbis->ReadMatrix4();
            

            pglobset->aglob[i].dmat.push_back(instanceModelMatrix);
        }
        
        if ((unk_5 & 2) != 0)
            pglobset->aglobi[i].grfzon = pbis->U32Read();

        if ((unk_5 & 0x200) != 0)
            pglobset->aglob[i].rSubglobRadius = pbis->F32Read();

        if ((unk_5 & 4) != 0)
        {
            float gZOrder = pbis->F32Read();

            if (gZOrder == 0x7F7FFFFF)
                pglobset->aglob[i].gZOrder = gZOrder;
            else
                pglobset->aglob[i].gZOrder = gZOrder * abs(gZOrder);
        }
        
        if ((unk_5 & 8) != 0)
            pglobset->aglob[i].uFog = pbis->F32Read();

        if ((unk_5 & 0x10) != 0)
        {
            pglobset->aglob[i].sMRD = pbis->F32Read();

            if (pglobset->aglob[i].sMRD == 3.402823e+38)
                pglobset->aglob[i].sMRD = 10000000000.000000;
        }
        
        if ((unk_5 & 0x20) != 0)
        {
            float sCelBorderMRD = pbis->F32Read();

            if (sCelBorderMRD == 3.402823e+38)
                pglobset->aglob[i].sCelBorderMRD = 2000.0;
            else
                pglobset->aglob[i].sCelBorderMRD = sCelBorderMRD;
        }

        if ((unk_5 & 0x40) != 0)
            PsaaLoadFromBrx(pbis);

        if ((unk_5 & 0x80) != 0)
        {
            GLEAM gleam{};
            gleam.normal  = pbis->ReadVector();

            gleam.clqc.g0 = pbis->F32Read();
            gleam.clqc.g1 = pbis->F32Read();
            gleam.clqc.g2 = pbis->F32Read();
            gleam.clqc.g3 = pbis->F32Read();

            pglobset->aglob[i].gleam.push_back(gleam);
        }

        if ((unk_5 & 0x100) != 0)
        {
            WRBG wrbg{};
            
            wrbg.palo = palo;
            wrbg.pglob = &pglobset->aglob[i];

            wrbg.oid = (OID)pbis->S16Read();
            wrbg.weki.wek = (WEK)pbis->S8Read();

            if (wrbg.weki.wek != WEK_Nil)
            {
                wrbg.weki.sInner = pbis->F32Read();
                wrbg.weki.uInner = pbis->F32Read();
                wrbg.weki.sOuter = pbis->F32Read();
                wrbg.weki.uOuter = pbis->F32Read();
                wrbg.weki.dmat   = pbis->ReadMatrix4();
            }

            wrbg.cmat  = pbis->U8Read();
            wrbg.fDpos = pbis->U8Read();
            wrbg.fDuv  = pbis->U8Read();
            wrbg.pwrbgNextGlobset = pglobset->pwrbgFirst;
            pglobset->aglob[i].wrbg.push_back(wrbg);
            pglobset->pwrbgFirst = &pglobset->aglob[i].wrbg[0];
        }
         
        pglobset->aglob[i].posCenter = pbis->ReadVector();
        pglobset->aglob[i].sRadius   = pbis->F32Read();
        pglobset->aglob[i].oid       = (OID)pbis->S16Read();
        pglobset->aglob[i].rtck      = (RTCK)pbis->U8Read();
        pglobset->aglob[i].rp        = (RP)pbis->U8Read();
        pglobset->aglob[i].grfglob   = pbis->U8Read();
        
        BuildCmFgfn(g_pcm, pglobset->aglob[i].uFog, &pglobset->aglob[i].fgfn);
        
        if (fCloneSubGlob == 0)
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
                uint32_t vertexCount = pbis->U8Read();
                //std::cout << std::dec << "Rotations Count: " << (uint32_t)rotationsCount << "\n";
                uint32_t normalCount = pbis->U8Read();
                //std::cout << std::dec << "Vertex Color Count: " << (uint32_t)vertexColorCount << "\n";
                uint32_t vertexColorCount = pbis->U8Read();
                //std::cout << std::dec << "Texcoords Count: " << (uint32_t)texCoordCount << "\n";
                uint32_t texcoordCount = pbis->U8Read();
                //std::cout << std::dec << "Index Count: " << (uint32_t)indexCount << "\n";
                uint32_t indexCount = pbis->U8Read();
                
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
                    indexes[f].ipos    = (uint32_t)pbis->U8Read();
                    indexes[f].inormal = (uint32_t)pbis->U8Read();
                    indexes[f].iuv     = (uint32_t)pbis->U8Read();
                    indexes[f].bMisc   = (uint32_t)pbis->U8Read();
                }

                // Loading texture property 
                pglobset->aglob[i].asubglob[a].pshd = &g_ashd[pbis->U16Read()];
                pglobset->aglob[i].asubglob[a].unSelfIllum = pbis->U8Read() * 0x7FA6 / 0xFF;
                pglobset->aglob[i].asubglob[a].cibnd = pbis->U8Read();

                pbis->file.seekg(pglobset->aglob[i].asubglob[a].cibnd, SEEK_CUR);

                int weightCount = vertexCount * pglobset->aglob[i].asubglob[a].cibnd;

                std::vector <float> agWeights;
                agWeights.resize(weightCount);

                for (int i = 0; i < weightCount; i++)
                    agWeights[i] = pbis->F32Read();

                std::vector <glm::vec3> posfPose;
                std::vector <glm::vec3> normalfPose;
                SUBPOSEF subposef;

                if (pglobset->cpose != 0)
                {
                    uint16_t posfPosesCount = pbis->U16Read();
                    posfPose.resize(posfPosesCount);

                    for (int i = 0; i < posfPosesCount; i++)
                        posfPose[i] = pbis->ReadVector();

                    uint16_t normalfPoseCount = pbis->U16Read();
                    normalfPose.resize(normalfPoseCount);

                    for (int i = 0; i < normalfPoseCount; i++)
                        normalfPose[i] = pbis->ReadVector();

                    subposef.aiposf.resize(indexCount);
                    subposef.ainormalf.resize(indexCount);

                    for (int i = 0; i < pglobset->cpose; i++)
                    {
                        for (int i = 0; i < indexCount; i++)
                            subposef.aiposf[i] = pbis->U16Read();

                        for (int i = 0; i < indexCount; i++)
                            subposef.ainormalf[i] = pbis->U16Read();
                    }
                }

                BuildSubGlob(&pglobset->aglob[i].asubglob[a] ,pglobset->aglob[i].asubglob[a].pshd, vertexes, normals, vertexColors, texcoords, indexes, &subposef, posfPose, normalfPose, agWeights);
            }

            pglobset->aglob[0].csubcel = pbis->U16Read();

            for (int i = 0; i < pglobset->aglob[0].csubcel; i++)
            {
                SUBCEL subcel;
                fCelBorder = 1;

                byte aposfCount = pbis->U8Read();

                std::vector <glm::vec3> aposf;
                aposf.resize(aposfCount);

                for (int a = 0; a < aposfCount; a++)
                    aposf[a] = pbis->ReadVector();

                byte ctwef = pbis->U8Read();

                std::vector <TWEF> atwef;
                atwef.resize(ctwef);

                for (int b = 0; b < ctwef; b++)
                {
                    atwef[b].aipos0 = (uint32_t)pbis->U8Read();
                    atwef[b].aipos1 = (uint32_t)pbis->U8Read();
                    atwef[b].aipos2 = (uint32_t)pbis->U8Read();
                    atwef[b].aipos3 = (uint32_t)pbis->U8Read();
                }

                int cibnd = pbis->U8Read();

                std::vector <int> aibnd;
                aibnd.resize(cibnd);

                for (int c = 0; c < cibnd; c++)
                    aibnd[c] = pbis->U8Read();

                int weightsCelCount = cibnd * aposfCount;

                std::vector <float> weightsCel;
                weightsCel.resize(weightsCelCount);

                for (int d = 0; d < weightsCelCount; d++)
                    weightsCel[d] = pbis->F32Read();

                std::vector <SUBPOSEF> subposef;
                std::vector <glm::vec3> aposfPoses;

                if (pglobset->cpose != 0)
                {
                    uint16_t aposfPosesCount = pbis->U16Read();
                    aposfPoses.resize(aposfPosesCount);

                    for (int i = 0; i < aposfPosesCount; i++)
                        aposfPoses[i] = pbis->ReadVector();

                    subposef.resize(pglobset->cpose);
                    
                    for (int i = 0; i < pglobset->cpose; i++)
                    {
                        subposef[i].aiposf.resize(aposfCount);

                        for (int a = 0; a < aposfCount; a++)
                            subposef[i].aiposf[a] = pbis->U16Read();
                    }
                }
                
                //BuildSubcel(pglobset, &subcel, aposfCount, aposf, ctwef, atwef, subposef, aposfPoses, weightsCel);

                pglobset->aglob[0].asubcel.push_back(subcel);
            }
        }
        else
        {
            pglobset->aglob[i].csubglob = pglobset->aglob[pglobset->aglob[i].instanceIndex].csubglob;
            pglobset->aglob[i].asubglob = pglobset->aglob[pglobset->aglob[i].instanceIndex].asubglob;
        }
    }

    if (pglobset->aglob.size() != 0)
    {
        if (pglobset->aglob[0].asubcel.size() != 0)
        {
            for (int i = 0; i < pglobset->aglob.size(); i++)
            {
                if (pglobset->aglob[i].rp != RP_Cutout && pglobset->aglob[i].rp != RP_ProjVolume && pglobset->aglob[i].rp != RP_CutoutAfterProjVolume && pglobset->aglob[i].rp != RP_Translucent)
                {
                    for (int a = 0; a < pglobset->aglob[i].asubglob.size(); a++)
                    {
                        for (int b = 0; b < pglobset->aglob[i].asubglob[a].vertices.size(); b++)
                        {
                            glm::vec3 newPos;

                            if (cid == 94)
                                newPos = pglobset->aglob[i].asubglob[a].vertices[b].pos * glm::vec3(1.1);
                            else if (cid == 95)
                                newPos = pglobset->aglob[i].asubglob[a].vertices[b].pos * glm::vec3(1.1);
                            else if (cid == 96)
                                newPos = pglobset->aglob[i].asubglob[a].vertices[b].pos * glm::vec3(1.1);
                            else
                                newPos = pglobset->aglob[i].asubglob[a].vertices[b].pos + pglobset->aglob[i].asubglob[a].vertices[b].normal * glm::vec3(3.0);

                            pglobset->aglob[i].asubglob[a].celPositions.push_back(glm::vec3(newPos));
                        }

                        pglobset->aglob[i].asubglob[a].celIndices = pglobset->aglob[i].asubglob[a].indices;

                        pglobset->aglob[i].asubglob[a].celcvtx = pglobset->aglob[i].asubglob[a].celIndices.size() * sizeof(INDICE);
                        pglobset->aglob[i].asubglob[a].fCelBorder = 1;

                        glGenVertexArrays(1, &pglobset->aglob[i].asubglob[a].celVAO);
                        glBindVertexArray(pglobset->aglob[i].asubglob[a].celVAO);

                        glGenBuffers(1, &pglobset->aglob[i].asubglob[a].celVBO);
                        glBindBuffer(GL_ARRAY_BUFFER, pglobset->aglob[i].asubglob[a].celVBO);
                        glBufferData(GL_ARRAY_BUFFER, pglobset->aglob[i].asubglob[a].celPositions.size() * sizeof(glm::vec3), pglobset->aglob[i].asubglob[a].celPositions.data(), GL_STATIC_DRAW);

                        glGenBuffers(1, &pglobset->aglob[i].asubglob[a].EBO);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pglobset->aglob[i].asubglob[a].EBO);
                        glBufferData(GL_ELEMENT_ARRAY_BUFFER, pglobset->aglob[i].asubglob[a].celIndices.size() * sizeof(INDICE), pglobset->aglob[i].asubglob[a].celIndices.data(), GL_STATIC_DRAW);

                        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
                        glEnableVertexAttribArray(0);
                    }
                }
            }
        }
    }
}

void BuildSubGlob(SUBGLOB *psubglob, SHD *pshd, std::vector <glm::vec3> &positions, std::vector <glm::vec3> &normals, std::vector <glm::vec4> &colors, std::vector <glm::vec2> &texcoords, std::vector <VTXFLG> &indexes, SUBPOSEF *subposef, std::vector <glm::vec3> &aposfPoses, std::vector <glm::vec3> &anormalfPoses, std::vector <float> &agWeights)
{
    psubglob->vertices.resize(indexes.size());
    
    for (int i = 0; i < indexes.size(); i++)
    {
        psubglob->vertices[i].pos = positions[indexes[i].ipos];

        if (indexes[i].inormal == 0xFF)
            psubglob->vertices[i].normal = glm::vec3(0.0);
        else
            psubglob->vertices[i].normal = normals[indexes[i].inormal];

        if ((indexes[i].bMisc & 0x7F) == 0x7F)
            psubglob->vertices[i].color = pshd->rgba;
        else
            psubglob->vertices[i].color = colors[indexes[i].bMisc & 0x7F] * pshd->rgba;

        if (indexes[i].iuv == 0xFF)
            psubglob->vertices[i].uv = glm::vec2{0.0};
        else
            psubglob->vertices[i].uv = texcoords[indexes[i].iuv];
    }

    uint32_t idx = 0;
    for (int i = 2; i < indexes.size(); i++)
    {
        if (!(indexes[idx + 2].bMisc & 0x80))
        {
            if (i % 2 == 0)
            {
                INDICE indice;

                indice.v1 = idx + 0;
                indice.v2 = idx + 1;
                indice.v3 = idx + 2;

                psubglob->indices.push_back(indice);
            }
            else
            {
                INDICE indice;

                indice.v1 = idx + 0;
                indice.v2 = idx + 2;
                indice.v3 = idx + 1;

                psubglob->indices.push_back(indice);
            }
        }
        idx++;
    }

    psubglob->cvtx = psubglob->indices.size() * sizeof(INDICE);
    
    glGenVertexArrays(1, &psubglob->VAO);
    glBindVertexArray(psubglob->VAO);

    glGenBuffers(1, &psubglob->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, psubglob->VBO);
    glBufferData(GL_ARRAY_BUFFER, psubglob->vertices.size() * sizeof(VERTICE), psubglob->vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &psubglob->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, psubglob->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, psubglob->indices.size() * sizeof(INDICE), psubglob->indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)offsetof(VERTICE, pos));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)offsetof(VERTICE, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)offsetof(VERTICE, color));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)offsetof(VERTICE, uv));
    glEnableVertexAttribArray(3);
}

void BuildSubcel(GLOBSET *pglobset, SUBCEL *psubcel, int cposf, std::vector <glm::vec3> &aposf, int ctwef, std::vector <TWEF> &atwef, std::vector <SUBPOSEF> &asubposef, std::vector <glm::vec3> &aposfPoses, std::vector <float> &agWeights)
{
    psubcel->weights = agWeights;
    psubcel->positions = aposf;

    for (int i = 0; i < ctwef; i++)
    {
        psubcel->indices.push_back(atwef[i].aipos0);
        psubcel->indices.push_back(atwef[i].aipos1);
        psubcel->indices.push_back(atwef[i].aipos2);
        psubcel->indices.push_back(atwef[i].aipos3);
    }
    
    std::vector <glm::vec3> unkVector;
    for (int i = 0; i < pglobset->cpose; i++)
    {
        for (int a = 0; a < cposf; a++)
            unkVector.push_back(aposfPoses[asubposef[i].aiposf[i]] - aposf[a]);
    }

    psubcel->cvtx = psubcel->indices.size() * sizeof(uint16_t);

    glGenVertexArrays(1, &psubcel->VAO);
    glBindVertexArray(psubcel->VAO);

    glGenBuffers(1, &psubcel->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, psubcel->VBO);
    glBufferData(GL_ARRAY_BUFFER, psubcel->positions.size() * sizeof(glm::vec3), psubcel->positions.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &psubcel->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, psubcel->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, psubcel->indices.size() * sizeof(uint16_t), psubcel->indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
}