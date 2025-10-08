#include "glob.h"

void LoadGlobsetFromBrx(GLOBSET *pglobset, ALO *palo, CBinaryInputStream *pbis)
{
    pglobset->cpsaa = 0;

    byte fRelight = pbis->U8Read();

    pglobset->cbnd = pbis->U8Read();
    pglobset->abnd.resize(pglobset->cbnd);

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
    int instanceIndex = 0;

    // Loading each submodel for a model
    for (int i = 0; i < pglobset->cglob; i++)
    {
        uint16_t unk_5 = pbis->U16Read();
        
        if ((unk_5 & 1) == 0)
        {
            fCloneSubGlob = 0;

            pglobset->aglob[i].sMRD = 1e+10;
            pglobset->aglob[i].sCelBorderMRD = 2000.0;
            pglobset->aglob[i].gZOrder = FLT_MAX;
            pglobset->aglob[i].uFog = 1.0;
            pglobset->aglob[i].rSubglobRadius = 1.0;
            pglobset->aglob[i].fDynamic = fRelight;
            pglobset->aglobi[i].uAlpha = 1.0;
        }

        else
        {
            fCloneSubGlob = unk_5 & 1;

            instanceIndex = pbis->S16Read();
            pglobset->aglob[i].posCenter = pglobset->aglob[instanceIndex].posCenter;
            pglobset->aglob[i].sRadius = pglobset->aglob[instanceIndex].sRadius;
            pglobset->aglob[i].rp = pglobset->aglob[instanceIndex].rp;
            pglobset->aglob[i].fThreeWay = pglobset->aglob[instanceIndex].fThreeWay;
            pglobset->aglob[i].sMRD = pglobset->aglob[instanceIndex].sMRD;
            pglobset->aglobi[i] = pglobset->aglobi[instanceIndex];

            glm::mat4 instanceModelMatrix = pbis->ReadMatrix4();

            std::shared_ptr <glm::mat4> mat = std::make_shared <glm::mat4> (instanceModelMatrix);
            pglobset->aglob[i].pdmat = mat;
        }
        
        if ((unk_5 & 2) != 0)
            pglobset->aglobi[i].grfzon = pbis->U32Read();
        else
            pglobset->aglobi[i].grfzon = -1;

        if ((unk_5 & 0x200) != 0)
            pglobset->aglob[i].rSubglobRadius = pbis->F32Read();

        if ((unk_5 & 4) != 0)
        {
            float gZOrder = pbis->F32Read();

            if (gZOrder == 3.402823e+38)
                pglobset->aglob[i].gZOrder = 3.402823e+38;
            else
                pglobset->aglob[i].gZOrder = gZOrder * glm::abs(gZOrder);
        }
        
        if ((unk_5 & 8) != 0)
            pglobset->aglob[i].uFog = pbis->F32Read();

        if ((unk_5 & 0x10) != 0)
        {
            pglobset->aglob[i].sMRD = pbis->F32Read();

            if (pglobset->aglob[i].sMRD == 3.402823e+38)
                pglobset->aglob[i].sMRD = 1e+10;
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

        if (fCloneSubGlob == 0)
        {
            // Number of submodels
            // std::cout << "Model Start: " << std::hex << file.tellg()<<"\n";
            pglobset->aglob[i].csubglob = pbis->U16Read();
            pglobset->aglob[i].asubglob.resize(pglobset->aglob[i].csubglob);

            numRo += pglobset->aglob[i].csubglob;

            for (int a = 0; a < pglobset->aglob[i].csubglob; a++)
            {
                // Loading submodel origin
                pglobset->aglob[i].asubglob[a].posCenter = pbis->ReadVector();
                pglobset->aglob[i].asubglob[a].sRadius = pbis->F32Read();

                //std::cout << std::dec << "Vertex Count: " << (uint32_t)vertexCount << "\n";
                uint32_t vertexCount = pbis->U8Read();
                //std::cout << std::dec << "Normal Count: " << (uint32_t)rotationsCount << "\n";
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

                pglobset->aglob[i].asubglob[a].shdID = pbis->U16Read();
                // Loading texture property 
                pglobset->aglob[i].asubglob[a].pshd = &g_ashd[pglobset->aglob[i].asubglob[a].shdID];
                pglobset->aglob[i].asubglob[a].unSelfIllum = pbis->U8Read() * 0x7FA6 / 0xFF;
                pglobset->aglob[i].asubglob[a].cibnd = pbis->U8Read();

                pglobset->aglob[i].asubglob[a].aibnd.resize(pglobset->aglob[i].asubglob[a].cibnd);

                for (int g = 0; g < pglobset->aglob[i].asubglob[a].cibnd; g++)
                    pglobset->aglob[i].asubglob[a].aibnd[g] = pbis->U8Read();

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

                    for (int g = 0; g < posfPosesCount; g++)
                        posfPose[g] = pbis->ReadVector();

                    uint16_t normalfPoseCount = pbis->U16Read();
                    normalfPose.resize(normalfPoseCount);

                    for (int h = 0; h < normalfPoseCount; h++)
                        normalfPose[h] = pbis->ReadVector();

                    subposef.aiposf.resize(indexCount);
                    subposef.ainormalf.resize(indexCount);

                    for (int j = 0; j < pglobset->cpose; j++)
                    {
                        for (int a = 0; a < indexCount; a++)
                            subposef.aiposf[a] = pbis->U16Read();

                        for (int b = 0; b < indexCount; b++)
                            subposef.ainormalf[b] = pbis->U16Read();
                    }
                }

                BuildSubGlob(&pglobset->aglob[i].asubglob[a] ,pglobset->aglob[i].asubglob[a].pshd, vertexes, normals, vertexColors, texcoords, indexes, &subposef, posfPose, normalfPose, agWeights);
            }

            pglobset->aglob[i].csubcel = pbis->U16Read();
            pglobset->aglob[i].asubcel.resize(pglobset->aglob[i].csubcel);

            numRo += pglobset->aglob[i].csubcel;

            for (int k = 0; k < pglobset->aglob[i].csubcel; k++)
            {
                SUBCEL subcel;

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

                    for (int e = 0; e < aposfPosesCount; e++)
                        aposfPoses[e] = pbis->ReadVector();

                    subposef.resize(pglobset->cpose);

                    for (int f = 0; f < pglobset->cpose; f++)
                    {
                        subposef[f].aiposf.resize(aposfCount);

                        for (int a = 0; a < aposfCount; a++)
                            subposef[f].aiposf[a] = pbis->U16Read();
                    }
                }

                BuildSubcel(pglobset, &subcel, aposfCount, aposf, ctwef, atwef, subposef, aposfPoses, weightsCel);
                pglobset->aglob[i].asubcel[k] = subcel;
            }
        }
        else
        {
            pglobset->aglob[i].csubglob = pglobset->aglob[instanceIndex].csubglob;
            pglobset->aglob[i].asubglob = pglobset->aglob[instanceIndex].asubglob;

            numRo += pglobset->aglob[instanceIndex].csubglob;
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

    const int maxInfluences = 4; // Assume 4 for GPU skinning
    const int cibnd = psubglob->cibnd; // number of influences per vertex
    const std::vector<int>& aibnd = psubglob->aibnd;
    const int vertexCount = indexes.size();

    for (int i = 0; i < vertexCount; i++)
    {
        int ipos = indexes[i].ipos;
        glm::uvec4 boneIDs(0);
        glm::vec4 weights(0.0f);

        for (int j = 0; j < cibnd && j < maxInfluences; j++)
        {
            int weightIndex = ipos * cibnd + j;

            if (weightIndex < agWeights.size())
            {
                weights[j] = agWeights[weightIndex];

                // aibnd maps influence slot j to real bone ID
                if (j < aibnd.size())
                    boneIDs[j] = aibnd[j];
            }
        }

        // Normalize weights
        float totalWeight = glm::compAdd(weights);
        if (totalWeight > 0.0f)
            weights /= totalWeight;

        psubglob->vertices[i].boneIndices = boneIDs;
        psubglob->vertices[i].boneWeights = weights;
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

    glVertexAttribIPointer(4, 4, GL_UNSIGNED_INT, sizeof(VERTICE), (void*)offsetof(VERTICE, boneIndices));
    glEnableVertexAttribArray(4);

    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)offsetof(VERTICE, boneWeights));
    glEnableVertexAttribArray(5);
}

void BuildSubcel(GLOBSET* pglobset, SUBCEL* psubcel, int cposf, std::vector <glm::vec3>& aposf, int ctwef, std::vector <TWEF>& atwef, std::vector <SUBPOSEF>& asubposef, std::vector <glm::vec3>& aposfPoses, std::vector <float>& agWeights)
{
    psubcel->positions = aposf;
    psubcel->edgeCount = static_cast<GLsizei>(ctwef);

    if (psubcel->edgeCount == 0) return;

    std::vector<glm::vec4> edgeTexels;
    edgeTexels.reserve(psubcel->edgeCount * 4);

    auto getP = [&](uint32_t idx)->const glm::vec3& {
        // (Optional) add asserts in debug builds
        return psubcel->positions[idx];
        };

    for (int i = 0; i < ctwef; ++i)
    {
        const uint32_t iOppA = atwef[i].aipos0; // opposite A
        const uint32_t iE0 = atwef[i].aipos1; // edge endpoint 0
        const uint32_t iE1 = atwef[i].aipos2; // edge endpoint 1
        const uint32_t iOppB = atwef[i].aipos3; // opposite B (may degenerate)

        const glm::vec3 e0 = getP(iE0);
        const glm::vec3 e1 = getP(iE1);
        const glm::vec3 oppA = getP(iOppA);
        const glm::vec3 oppB = getP(iOppB);

        // Pack 4 texels per edge, OBJECT-SPACE positions (w unused except kept as 1.0)
        edgeTexels.emplace_back(e0, 1.0f); // texel 0: E0
        edgeTexels.emplace_back(e1, 1.0f); // texel 1: E1
        edgeTexels.emplace_back(oppA, 1.0f); // texel 2: OppA
        edgeTexels.emplace_back(oppB, 1.0f); // texel 3: OppB
    }

    // Upload once (static)
    glGenBuffers(1, &psubcel->edgeBuf);
    glBindBuffer(GL_TEXTURE_BUFFER, psubcel->edgeBuf);
    glBufferData(GL_TEXTURE_BUFFER, edgeTexels.size() * sizeof(glm::vec4), edgeTexels.data(), GL_STATIC_DRAW);

    glGenTextures(1, &psubcel->edgeTex);
    glBindTexture(GL_TEXTURE_BUFFER, psubcel->edgeTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, psubcel->edgeBuf);
}

int  g_fogType = 1;
bool g_fRenderModels = true;
bool g_fRenderCollision = false;
bool g_fRenderCelBorders = true;
bool g_fBsp = false;
float g_uAlpha = 1.0;
GLuint gEmptyVAO = 0;