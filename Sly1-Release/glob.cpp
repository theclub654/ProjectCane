#include "glob.h"

void InitGlslUniforms()
{
    glslNumLights = glGetUniformLocation(glGlobShader.ID, "numLights");
    glslmatWorldToClip = glGetUniformLocation(glGlobShader.ID, "matWorldToClip");
    glslCameraPos = glGetUniformLocation(glGlobShader.ID, "cameraPos");

    glslLsmShadow = glGetUniformLocation(glGlobShader.ID, "lsm.uShadow");
    glslLsmDiffuse = glGetUniformLocation(glGlobShader.ID, "lsm.uMidtone");

    glslFogType = glGetUniformLocation(glGlobShader.ID, "fogType");
    glslFogNear = glGetUniformLocation(glGlobShader.ID, "fogNear");
    glslFogFar = glGetUniformLocation(glGlobShader.ID, "fogFar");
    glslFogMax = glGetUniformLocation(glGlobShader.ID, "fogMax");
    glslFogColor = glGetUniformLocation(glGlobShader.ID, "fogColor");

    glslRgbaCel = glGetUniformLocation(glGlobShader.ID, "rgbaCel");

    glslModel = glGetUniformLocation(glGlobShader.ID, "model");
    glslUFog = glGetUniformLocation(glGlobShader.ID, "uFog");
    glslUAlpha = glGetUniformLocation(glGlobShader.ID, "uAlpha");

    glslRDarken = glGetUniformLocation(glGlobShader.ID, "rDarken");
    glslRko = glGetUniformLocation(glGlobShader.ID, "rko");
    glslusSelfIllum = glGetUniformLocation(glGlobShader.ID, "usSelfIllum");
    glslFDynamic = glGetUniformLocation(glGlobShader.ID, "fDynamic");
    glslPosCenter = glGetUniformLocation(glGlobShader.ID, "posCenter");
    glslfAlphaTest = glGetUniformLocation(glGlobShader.ID, "fAlphaTest");

    glslCollisionRgba = glGetUniformLocation(glGlobShader.ID, "collisionRgba");

    glUniform1i(glGetUniformLocation(glGlobShader.ID, "shadowMap"), 0);
    glUniform1i(glGetUniformLocation(glGlobShader.ID, "diffuseMap"), 1);
    glUniform1i(glGetUniformLocation(glGlobShader.ID, "saturateMap"), 2);
}

void LoadGlobsetFromBrx(GLOBSET* pglobset, short cid, ALO* palo, CBinaryInputStream* pbis)
{
    pglobset->cpsaa = 0;

    byte fRelight = pbis->U8Read();
    pbis->U8Read();
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
    int fCelBorder = 0;
    int instanceIndex = 0;

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

            instanceIndex = pbis->S16Read();
            pglobset->aglob[i].instanceIndex = instanceIndex;
            pglobset->aglob[i].posCenter = pglobset->aglob[instanceIndex].posCenter;
            pglobset->aglob[i].sRadius = pglobset->aglob[instanceIndex].sRadius;
            pglobset->aglob[i].rp = pglobset->aglob[instanceIndex].rp;
            pglobset->aglob[i].fThreeWay = pglobset->aglob[instanceIndex].fThreeWay;
            pglobset->aglob[i].sMRD = pglobset->aglob[instanceIndex].sMRD;
            pglobset->aglobi[i] = pglobset->aglobi[instanceIndex];

            glm::mat4 instanceModelMatrix = pbis->ReadMatrix4();

            std::shared_ptr <glm::mat4> mat = std::make_shared <glm::mat4>(instanceModelMatrix);
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
                    indexes[f].ipos    = pbis->U8Read();
                    indexes[f].inormal = pbis->U8Read();
                    indexes[f].iuv     = pbis->U8Read();
                    indexes[f].bMisc   = pbis->U8Read();
                }

                // Loading texture property 
                pglobset->aglob[i].asubglob[a].pshd = &g_ashd[pbis->U16Read()];
                pglobset->aglob[i].asubglob[a].unSelfIllum = pbis->U8Read() * 0x7FA6 / 0xFF;
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
                
                BuildSubGlob(&pglobset->aglob[i].asubglob[a], pglobset->aglob[i].asubglob[a].pshd, vertexes, normals, vertexColors, texcoords, indexes);
            }

            uint16_t numSubMesh1 = pbis->U16Read();

            for (int i = 0; i < numSubMesh1; i++)
            {
                fCelBorder = 1;
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
        else
        {
            pglobset->aglob[i].csubglob = pglobset->aglob[pglobset->aglob[i].instanceIndex].csubglob;
            pglobset->aglob[i].asubglob = pglobset->aglob[pglobset->aglob[i].instanceIndex].asubglob;

            numRo += pglobset->aglob[instanceIndex].csubglob;
        }
    }
    
    if (!pglobset->aglob.empty() && fCelBorder != 0)
    {
        for (int i = 0; i < pglobset->aglob.size(); i++)
        {
            const auto& glob = pglobset->aglob[i];

            // Skip excluded render passes
            if (glob.rp == RP_Cutout || glob.rp == RP_ProjVolume ||
                glob.rp == RP_CutoutAfterProjVolume || glob.rp == RP_Translucent)
                continue;

            for (int a = 0; a < glob.asubglob.size(); a++)
            {
                auto& subglob = pglobset->aglob[i].asubglob[a];
                const glm::vec3& center = subglob.posCenter;

                constexpr float thickness = 4.0f;

                for (const auto& vert : subglob.vertices)
                {
                    const glm::vec3& pos = vert.pos;

                    // Inflate outward from posCenter
                    glm::vec3 offsetDir = glm::normalize(pos - center);
                    glm::vec3 newPos = pos + offsetDir * thickness;

                    subglob.celPositions.push_back(newPos);
                }

                subglob.celIndices = subglob.indices;
                subglob.celcvtx = subglob.celIndices.size() * sizeof(INDICE);
                subglob.fCelBorder = 1;

                glGenVertexArrays(1, &subglob.celVAO);
                glBindVertexArray(subglob.celVAO);

                glGenBuffers(1, &subglob.celVBO);
                glBindBuffer(GL_ARRAY_BUFFER, subglob.celVBO);
                glBufferData(GL_ARRAY_BUFFER, subglob.celPositions.size() * sizeof(glm::vec3), subglob.celPositions.data(), GL_STATIC_DRAW);

                glGenBuffers(1, &subglob.EBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subglob.EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, subglob.celIndices.size() * sizeof(INDICE), subglob.celIndices.data(), GL_STATIC_DRAW);

                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
                glEnableVertexAttribArray(0);
            }
        }
    }
}

void BuildSubGlob(SUBGLOB *psubglob, SHD *pshd, std::vector <glm::vec3> &positions, std::vector <glm::vec3> &normals, std::vector <glm::vec4> &colors, std::vector <glm::vec2> &texcoords, std::vector <VTXFLG> &indexes)
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

int  g_fogType = 1;
bool g_fRenderModels = true;
bool g_fRenderCollision = false;
bool g_fRenderCelBorders = true;
bool g_fBsp = false;

GLuint glslNumLights = 0;
GLuint glslmatWorldToClip = 0;
GLuint glslCameraPos = 0;
GLuint glslFogType = 0;
GLuint glslFogNear = 0;
GLuint glslFogFar = 0;
GLuint glslFogMax = 0;
GLuint glslFogColor = 0;
GLuint glslLsmShadow = 0;
GLuint glslLsmDiffuse = 0;
GLuint glslRgbaCel = 0;
GLuint glslModel = 0;
GLuint glslUFog = 0;
GLuint glslUAlpha = 0;
GLuint glslRDarken = 0;
GLuint glslRko = 0;
GLuint glslusSelfIllum = 0;
GLuint glslFDynamic = 0;
GLuint glslPosCenter = 0;
GLuint glslfAlphaTest = 0;
GLuint glslCollisionRgba = 0;