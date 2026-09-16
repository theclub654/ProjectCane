#include "glob.h"
#include "wr.h"
#include "render.h"

void LoadGlobsetFromBrx(GLOBSET* pglobset, ALO* palo, CBinaryInputStream* pbis)
{
    pglobset->cpsaa = 0;

    byte fRelight = pbis->U8Read();
    pbis->U8Read();

    pglobset->cbnd = pbis->U8Read();
    pglobset->abnd.resize(pglobset->cbnd);

    pglobset->mpibndoid.resize(pglobset->cbnd);
    for (int i = 0; i < pglobset->cbnd; i++)
        pglobset->mpibndoid[i] = (OID)pbis->S16Read();

    pglobset->cpose = pbis->U8Read();

    pglobset->agPoses.resize(pglobset->cpose);

    for (int i = 0; i < pglobset->cpose; ++i)
        pglobset->agPoses[i] = pbis->F32Read();

    pglobset->agPosesOrig = pglobset->agPoses;

    // Loading number of submodels for model
    pglobset->cglob = pbis->U16Read();

    pglobset->aglob.resize(pglobset->cglob);
    pglobset->aglobi.resize(pglobset->cglob);

    int fInstanceGlob = 0;
    int instanceIndex = 0;

    // Loading each submodel for a model
    for (int i = 0; i < pglobset->cglob; i++)
    {
        uint16_t globPropertys = pbis->U16Read();

        if ((globPropertys & 0x0001) == 0)
        {
            fInstanceGlob = 0;

            pglobset->aglob[i].sMRD = 1.0e10f;
            pglobset->aglob[i].sCelBorderMRD = 2000.0;
            pglobset->aglob[i].gZOrder = std::numeric_limits<float>::max();
            pglobset->aglob[i].uFog = 1.0;
            pglobset->aglob[i].rSubglobRadius = 1.0;
            pglobset->aglob[i].fDynamic = fRelight;
            pglobset->aglob[i].fTransluscentSort = 0;
            pglobset->aglobi[i].uAlpha = 1.0;
        }
        else
        {
            fInstanceGlob = globPropertys & 1;
            instanceIndex = pbis->S16Read();

            pglobset->aglob[i].VAO = pglobset->aglob[instanceIndex].VAO;
            pglobset->aglob[i].VBO = pglobset->aglob[instanceIndex].VBO;
            pglobset->aglob[i].EBO = pglobset->aglob[instanceIndex].EBO;
            pglobset->aglob[i].posCenter = pglobset->aglob[instanceIndex].posCenter;
            pglobset->aglob[i].sRadius = pglobset->aglob[instanceIndex].sRadius;
            pglobset->aglob[i].rp = pglobset->aglob[instanceIndex].rp;
            pglobset->aglob[i].sMRD = pglobset->aglob[instanceIndex].sMRD;
            pglobset->aglob[i].sCelBorderMRD = pglobset->aglob[instanceIndex].sCelBorderMRD;
            pglobset->aglob[i].gZOrder = pglobset->aglob[instanceIndex].gZOrder;
            pglobset->aglob[i].uFog = pglobset->aglob[instanceIndex].uFog;
            pglobset->aglob[i].fDynamic = pglobset->aglob[instanceIndex].fDynamic;
            pglobset->aglob[i].fTransluscentSort = pglobset->aglob[instanceIndex].fTransluscentSort;
            pglobset->aglob[i].fThreeWay = pglobset->aglob[instanceIndex].fThreeWay;
            pglobset->aglob[i].trlk = pglobset->aglob[instanceIndex].trlk;
            pglobset->aglob[i].grfshd = pglobset->aglob[instanceIndex].grfshd;
            pglobset->aglobi[i] = pglobset->aglobi[instanceIndex];

            glm::mat4 instanceModelMatrix = pbis->ReadMatrix4();

            std::shared_ptr <glm::mat4> mat = std::make_shared <glm::mat4>(instanceModelMatrix);
            pglobset->aglob[i].pdmat = mat;
        }

        if ((globPropertys & 2) != 0)
            pglobset->aglobi[i].grfzon = pbis->U32Read();

        if ((globPropertys & 0x200) != 0)
            pglobset->aglob[i].rSubglobRadius = pbis->F32Read();

        if (globPropertys & 0x04)
        {
            float gZOrder = pbis->F32Read();

            if (gZOrder == std::numeric_limits<float>::max())
                pglobset->aglob[i].gZOrder = gZOrder;
            else
                pglobset->aglob[i].gZOrder = gZOrder * std::abs(gZOrder);
        }

        if ((globPropertys & 8) != 0)
            pglobset->aglob[i].uFog = pbis->F32Read();

        if (globPropertys & 0x10)
        {
            const float v = pbis->F32Read();
            pglobset->aglob[i].sMRD = (v == std::numeric_limits<float>::max()) ? 1.0e10f : v;
        }

        if (globPropertys & 0x20)
        {
            float sCelBorderMRD = pbis->F32Read();
            pglobset->aglob[i].sCelBorderMRD = (sCelBorderMRD == std::numeric_limits<float>::max()) ? 2000.0f : sCelBorderMRD;
        }

        // Clamp afterward
        pglobset->aglob[i].sCelBorderMRD = std::min(pglobset->aglob[i].sCelBorderMRD, pglobset->aglob[i].sMRD);

        if ((globPropertys & 0x40) != 0)
        {
            pglobset->aglob[i].psaa = PsaaLoadFromBrx(pbis);

            if (pglobset->aglob[i].psaa != nullptr)
                pglobset->cpsaa++;
        }

        if ((globPropertys & 0x80) != 0)
        {
            GLEAM gleam{};
            gleam.normal = pbis->ReadVector();

            gleam.clqc.g0 = pbis->F32Read();
            gleam.clqc.g1 = pbis->F32Read();
            gleam.clqc.g2 = pbis->F32Read();
            gleam.clqc.g3 = pbis->F32Read();

            std::shared_ptr <GLEAM> pgleam = std::make_shared <GLEAM>(gleam);
            pglobset->aglob[i].gleam = pgleam;
        }

        if ((globPropertys & 0x100) != 0)
        {
            auto wrbgPtr = std::make_shared<WRBG>();
            WRBG& wrbg = *wrbgPtr;

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

            // preserve original (char) cast behavior:
            wrbg.cmat  = (int8_t)pbis->U8Read();
            wrbg.fDpos = (int8_t)pbis->U8Read();
            wrbg.fDuv  = (int8_t)pbis->U8Read();

            if (wrbg.fDpos == 1 && wrbg.fDuv == 0)
                wrbg.warpType = WARP_POS;
            else if (wrbg.fDpos == 0 && wrbg.fDuv == 1)
                wrbg.warpType = WARP_UV;
            else if (wrbg.fDpos == 1 && wrbg.fDuv == 1)
                wrbg.warpType = WARP_BOTH;

            pglobset->aglob[i].pwrbg = wrbgPtr;
            wrbg.pwrbgNextGlobset    = pglobset->pwrbgFirst;
            pglobset->pwrbgFirst     = wrbgPtr;
        }

        pglobset->aglob[i].posCenter = pbis->ReadVector();
        pglobset->aglob[i].sRadius   = pbis->F32Read();
        pglobset->aglob[i].oid       = (OID)pbis->S16Read();
        pglobset->aglob[i].rtck      = (RTCK)pbis->U8Read();
        pglobset->aglob[i].rp        = (RP)pbis->U8Read();
        pglobset->aglob[i].grfglob   = pbis->U8Read();

        if (fInstanceGlob == 0)
        {
            GLOB& glob = pglobset->aglob[i];

            glob.poseCount = pglobset->cpose;
            glob.poseVertexCount = 0;
            glob.poseDpos.clear();
            glob.poseDnormal.clear();

            int fProjVolume = 0;
            // Number of submodels
            // std::cout << "Model Start: " << std::hex << file.tellg()<<"\n";
            pglobset->aglob[i].csubglob = pbis->U16Read();
            pglobset->aglob[i].asubglob.resize(pglobset->aglob[i].csubglob);
            pglobset->aglobi[i].asubglobi.resize(pglobset->aglob[i].csubglob);

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

                pbis->Align(0x4);

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
                pglobset->aglob[i].asubglob[a].shdID = pbis->U16Read();
                pglobset->aglob[i].asubglob[a].pshd  = &g_ashd[pglobset->aglob[i].asubglob[a].shdID];

                auto& glob = pglobset->aglob[i];
                auto* shd  = glob.asubglob[a].pshd;

                if (!glob.fTransluscentSort && shd)
                {
                    const uint32_t g = (uint32_t)shd->grfshd;

                    if (g == 2)
                    {
                        if (glob.rp == RP_Background ||
                            glob.rp == RP_Cutout ||
                            glob.rp == RP_CutoutAfterProjVolume ||
                            glob.rp == RP_Translucent)
                        {
                            glob.fTransluscentSort = 1;
                        }
                    }
                    else if (g == 6)
                    {
                        if (glob.rp == RP_Translucent)
                            glob.fTransluscentSort = 1;
                    }
                }

                if (glob.rp == RP_ProjVolume)
                    glob.grfshd = glob.asubglob[a].pshd->grfshd;

                pglobset->aglob[i].asubglob[a].unSelfIllum = static_cast<uint16_t>((pbis->U8Read() * 0x7FA6) / 0xFF);

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
                std::vector<SUBPOSEF> asubposef;

                if (pglobset->cpose != 0)
                {
                    uint16_t cposfPoses = pbis->U16Read();
                    posfPose.resize(cposfPoses);

                    for (int i = 0; i < cposfPoses; ++i)
                        posfPose[i] = pbis->ReadVector();

                    uint16_t cnormalfPoses = pbis->U16Read();
                    normalfPose.resize(cnormalfPoses);

                    for (int i = 0; i < cnormalfPoses; ++i)
                        normalfPose[i] = pbis->ReadVector();

                    asubposef.resize(pglobset->cpose);

                    for (int ipose = 0; ipose < pglobset->cpose; ++ipose)
                    {
                        asubposef[ipose].aiposf.resize(indexCount);
                        asubposef[ipose].ainormalf.resize(indexCount);

                        for (int ivtx = 0; ivtx < indexCount; ++ivtx)
                            asubposef[ipose].aiposf[ivtx] = pbis->U16Read();

                        for (int ivtx = 0; ivtx < indexCount; ++ivtx)
                            asubposef[ipose].ainormalf[ivtx] = pbis->U16Read();
                    }
                }

                BuildSubGlob(&pglobset->aglob[i], &pglobset->aglob[i].asubglob[a], pglobset->aglob[i].asubglob[a].pshd, vertexes, normals, vertexColors, texcoords, indexes, asubposef, posfPose, normalfPose, agWeights, pglobset->aglob[i].fDynamic);
            }

            if (pglobset->aglob[i].asubglob.size() > 0)
            {
                auto& glob = pglobset->aglob[i];

                // 1) Count totals
                size_t totalVerts = 0;
                size_t totalIndices = 0; // flattened (tri*3)

                for (auto& s : glob.asubglob)
                {
                    totalVerts   += s.vertices.size();
                    totalIndices += s.indices.size() * 3;
                }

                // 2) Allocate final CPU arrays once (no insert() / no reallocation)
                std::vector <VERTICE>  packedVerts(totalVerts);
                std::vector <uint32_t> packedIdx(totalIndices);

                VERTICE*  vptr = packedVerts.data();
                uint32_t* iptr = packedIdx.data();

                uint32_t vOffset = 0;
                uint32_t iOffset = 0;

                // 3) Pack each subglob into the big arrays + record slice offsets
                for (auto& s : glob.asubglob)
                {
                    s.baseVertex = (int32_t)vOffset;
                    s.firstIndex = (uint32_t)iOffset;
                    s.indexCount = (uint32_t)(s.indices.size() * 3);

                    // Copy vertices
                    if (!s.vertices.empty())
                    {
                        memcpy(vptr, s.vertices.data(), s.vertices.size() * sizeof(VERTICE));
                        vptr += s.vertices.size();
                        vOffset += (uint32_t)s.vertices.size();
                    }

                    for (auto& tri : s.indices)
                    {
                        iptr[0] = (uint32_t)tri.v1;
                        iptr[1] = (uint32_t)tri.v2;
                        iptr[2] = (uint32_t)tri.v3;
                        iptr += 3;
                    }

                    iOffset += (uint32_t)(s.indices.size() * 3);
                }

                // 4) Create ONE VAO/VBO/EBO for the glob and upload once
                glGenVertexArrays(1, &glob.VAO);
                glBindVertexArray(glob.VAO);

                glGenBuffers(1, &glob.VBO);
                glBindBuffer(GL_ARRAY_BUFFER, glob.VBO);
                glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(packedVerts.size() * sizeof(VERTICE)), packedVerts.data(), GL_STATIC_DRAW);

                glGenBuffers(1, &glob.EBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glob.EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(packedIdx.size() * sizeof(uint32_t)), packedIdx.data(), GL_STATIC_DRAW);

                // 5) Vertex attributes ONCE (your exact layout)
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

                glBindVertexArray(0);

                glob.poseVertexCount = static_cast<int>(totalVerts);

                const size_t expectedPoseDeltaCount = static_cast<size_t>(glob.poseVertexCount) * glob.poseCount;

                assert(glob.poseDpos.size() == expectedPoseDeltaCount);
                assert(glob.poseDnormal.size() == expectedPoseDeltaCount);

                if (expectedPoseDeltaCount > 0)
                {
                    glGenBuffers(1, &glob.poseDposSSBO);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.poseDposSSBO);
                    glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(glob.poseDpos.size() * sizeof(glm::vec4)), glob.poseDpos.data(), GL_STATIC_DRAW);

                    glGenBuffers(1, &glob.poseDnormalSSBO);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.poseDnormalSSBO);
                    glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(glob.poseDnormal.size() * sizeof(glm::vec4)), glob.poseDnormal.data(), GL_STATIC_DRAW);

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
                }

                if (glob.pwrbg && glob.pwrbg->cmat > 0)
                {
                    if (!glob.pwarpGlob)
                        glob.pwarpGlob = std::make_shared<WRBGLOB_GL>();

                    WRBGLOB_GL& w = *glob.pwarpGlob;

                    w.vertexCount = (int)packedVerts.size();
                    w.basePos.resize((size_t)w.vertexCount);

                    for (int v = 0; v < w.vertexCount; ++v)
                        w.basePos[v] = glm::vec4(packedVerts[v].pos, 1.0f);

                    // don't allocate state yet here (because WR->cmat isn’t known until ApplyWrGlob)
                    // but you CAN create the SSBO name now:
                    if (w.ssboState == 0)
                        glGenBuffers(1, &w.ssboState);
                }

                if (glob.fThreeWay == 1 && glob.fDynamic == 0)
                {
                    if (glob.pwarpGlob == nullptr)
                    {
                        glob.trlk = TRLK_Relight;

                        glGenBuffers(1, &glob.ssboCachedMaterial);
                        glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.ssboCachedMaterial);
                        glBufferData(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)(totalVerts * sizeof(MATERIAL)), nullptr, GL_STATIC_DRAW);
                        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
                    }
                }

                SetGlobDraw(&pglobset->aglob[i]);
                numRo++;
            }

            pglobset->aglob[i].csubcel = pbis->U16Read();
            pglobset->aglob[i].asubcel.resize(pglobset->aglob[i].csubcel);

            std::vector<glm::vec4> mergedEdges;
            std::vector<glm::uvec4> mergedEdgeBoneIndices;
            std::vector<glm::vec4> mergedEdgeBoneWeights;
            std::vector<glm::vec4> mergedEdgePoseDpos;


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

                BuildSubcel(pglobset, &glob, &subcel, aposfCount, aposf, ctwef, atwef, cibnd, aibnd, subposef, aposfPoses, weightsCel, mergedEdges, mergedEdgeBoneIndices, mergedEdgeBoneWeights, mergedEdgePoseDpos);
                glob.asubcel[k] = std::move(subcel);
            }

            if (glob.edgeCount > 0)
            {
                const size_t expectedEdgePointCount = static_cast<size_t>(glob.edgeCount) * 4;
                const size_t expectedEdgePoseCount = expectedEdgePointCount * glob.poseCount;

                assert(mergedEdges.size() == expectedEdgePointCount);
                assert(mergedEdgeBoneIndices.size() == expectedEdgePointCount);
                assert(mergedEdgeBoneWeights.size() == expectedEdgePointCount);
                assert(mergedEdgePoseDpos.size() == expectedEdgePoseCount);

                glGenBuffers(1, &glob.edgeSSBO);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.edgeSSBO);
                glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(mergedEdges.size() * sizeof(glm::vec4)), mergedEdges.data(), GL_STATIC_DRAW);

                glGenBuffers(1, &glob.edgeBoneIndicesSSBO);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.edgeBoneIndicesSSBO);
                glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(mergedEdgeBoneIndices.size() * sizeof(glm::uvec4)), mergedEdgeBoneIndices.data(), GL_STATIC_DRAW);

                glGenBuffers(1, &glob.edgeBoneWeightsSSBO);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.edgeBoneWeightsSSBO);
                glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(mergedEdgeBoneWeights.size() * sizeof(glm::vec4)), mergedEdgeBoneWeights.data(), GL_STATIC_DRAW);

                if (!mergedEdgePoseDpos.empty())
                {
                    glGenBuffers(1, &glob.edgePoseDposSSBO);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.edgePoseDposSSBO);
                    glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(mergedEdgePoseDpos.size() * sizeof(glm::vec4)), mergedEdgePoseDpos.data(), GL_STATIC_DRAW);
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

                glob.edgeBoneIndices = std::move(mergedEdgeBoneIndices);
                glob.edgeBoneWeights = std::move(mergedEdgeBoneWeights);
                glob.edgePoseDpos = std::move(mergedEdgePoseDpos);

                numRoCel++;
            }
        }
        else
        {
            GLOB& inst = pglobset->aglob[i];
            GLOB& base = pglobset->aglob[instanceIndex];

            inst.csubglob = base.csubglob;
            inst.asubglob = base.asubglob;

            inst.csubcel = base.csubcel;
            inst.asubcel = base.asubcel;

            inst.edgeSSBO  = base.edgeSSBO;
            inst.edgeCount = base.edgeCount;

            inst.poseDposSSBO = base.poseDposSSBO;
            inst.poseDnormalSSBO = base.poseDnormalSSBO;
            inst.poseCount = base.poseCount;
            inst.poseVertexCount = base.poseVertexCount;

            CloneGlob(pglobset, &inst, &pglobset->aglobi[i]);
        }
    }

    BuildGlobsetSaaArray(pglobset);
}

void BuildSubGlob(GLOB* pglob, SUBGLOB* psubglob, SHD* pshd, std::vector <glm::vec3>& positions, std::vector <glm::vec3>& normals, std::vector <glm::vec4>& colors, std::vector <glm::vec2>& texcoords, std::vector <VTXFLG>& indexes, std::vector <SUBPOSEF>& subposef, std::vector <glm::vec3>& aposfPoses, std::vector <glm::vec3>& anormalfPoses, std::vector <float>& agWeights, int fDynamic)
{
    if (pshd->shdk == SHDK_ThreeWay)
        pglob->fThreeWay = 1;

    psubglob->vertices.resize(indexes.size());

    for (int i = 0; i < indexes.size(); i++)
    {
        psubglob->vertices[i].pos = positions[indexes[i].ipos];

        if (indexes[i].inormal == 0xFF)
            psubglob->vertices[i].normal = glm::vec3(0.0);
        else
            psubglob->vertices[i].normal = normals[indexes[i].inormal];

        if (pshd->shdk == SHDK_ProjectedVolume)
        {
            if ((indexes[i].bMisc & 0x7F) == 0x7F)
                psubglob->vertices[i].color = pshd->rgbaVolume;
            else
                psubglob->vertices[i].color = colors[indexes[i].bMisc & 0x7F] * pshd->rgbaVolume;
        }
        else
        {
            if ((indexes[i].bMisc & 0x7F) == 0x7F)
                psubglob->vertices[i].color = pshd->rgba;
            else
                psubglob->vertices[i].color = colors[indexes[i].bMisc & 0x7F] * pshd->rgba;
        }

        if (indexes[i].iuv == 0xFF)
            psubglob->vertices[i].uv = glm::vec2{ 0.0 };
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

        psubglob->vertices[i].boneIndices = boneIDs;
        psubglob->vertices[i].boneWeights = weights;
    }

    if (pglob->poseCount > 0)
    {
        const size_t vertexCount = indexes.size();

        pglob->poseDpos.reserve(pglob->poseDpos.size() + vertexCount * pglob->poseCount);
        pglob->poseDnormal.reserve(pglob->poseDnormal.size() + vertexCount * pglob->poseCount);

        for (size_t ivtx = 0; ivtx < vertexCount; ++ivtx)
        {
            const VTXFLG& vertex = indexes[ivtx];

            for (int ipose = 0; ipose < pglob->poseCount; ++ipose)
            {
                const SUBPOSEF& pose = subposef[ipose];

                glm::vec3 dpos(0.0f);
                glm::vec3 dnormal(0.0f);

                const uint16_t iposPose = pose.aiposf[ivtx];

                if (iposPose < aposfPoses.size() && vertex.ipos < positions.size())
                    dpos = aposfPoses[iposPose] - positions[vertex.ipos];

                if (vertex.inormal != 0xFF)
                {
                    const uint16_t inormalPose = pose.ainormalf[ivtx];

                    if (inormalPose < anormalfPoses.size() && vertex.inormal < normals.size())
                        dnormal = anormalfPoses[inormalPose] - normals[vertex.inormal];
                }

                pglob->poseDpos.emplace_back(dpos, 0.0f);
                pglob->poseDnormal.emplace_back(dnormal, 0.0f);
            }
        }

        pglob->poseVertexCount += static_cast<int>(vertexCount);
    }

    uint32_t idx = 0;
    for (int i = 2; i < indexes.size(); i++)
    {
        if (!(indexes[idx + 2].bMisc & 0x80))
        {
            if (i % 2 == 0)
            {
                INDICE indice{};

                indice.v1 = idx + 0;
                indice.v2 = idx + 1;
                indice.v3 = idx + 2;

                psubglob->indices.push_back(indice);
            }
            else
            {
                INDICE indice{};

                indice.v1 = idx + 0;
                indice.v2 = idx + 2;
                indice.v3 = idx + 1;

                psubglob->indices.push_back(indice);
            }
        }

        idx++;
    }

    SAI* uvSai = nullptr;
    bool usesUvAnim = false;

    SAI* sai = nullptr;

    if (pglob->psaa != nullptr)
        sai = pglob->psaa->pvtsaa->pfnPsaiFromSaaShd(pglob->psaa, pshd);

    if (!sai && pshd->psaa)
        sai = pshd->psaa->pvtsaa->pfnPsaiFromSaaShd(pshd->psaa, pshd);

    psubglob->uvSai = sai;
    psubglob->usesUvAnim = (sai && (sai->grfsai & 0x2));
}

void BuildSubcel(GLOBSET* pglobset, GLOB* pglob, SUBCEL* psubcel, int cposf, const std::vector<glm::vec3>& aposf, int ctwef, const std::vector<TWEF>& atwef, int cibnd, const std::vector<int>& aibnd, const std::vector<SUBPOSEF>& asubposef, const std::vector<glm::vec3>& aposfPoses, const std::vector<float>& agWeights, std::vector<glm::vec4>& totalEdges, std::vector<glm::uvec4>& totalBoneIndices, std::vector<glm::vec4>& totalBoneWeights, std::vector<glm::vec4>& totalPoseDpos)
{
    pglob->edgeCount += ctwef;

    const size_t pointCount = static_cast<size_t>(ctwef) * 4;

    totalEdges.reserve(totalEdges.size() + pointCount);
    totalBoneIndices.reserve(totalBoneIndices.size() + pointCount);
    totalBoneWeights.reserve(totalBoneWeights.size() + pointCount);
    totalPoseDpos.reserve(totalPoseDpos.size() + pointCount * pglobset->cpose);

    auto AppendCelPoint = [&](uint32_t ipos)
    {
        if (ipos >= aposf.size())
            return;

        glm::uvec4 boneIndices(0);
        glm::vec4 boneWeights(0.0f);

        const int influenceCount = std::min(cibnd, 4);

        for (int iibnd = 0; iibnd < influenceCount; ++iibnd)
        {
            const size_t weightIndex = static_cast<size_t>(ipos) * cibnd + iibnd;

            if (iibnd < static_cast<int>(aibnd.size()) && weightIndex < agWeights.size())
            {
                boneIndices[iibnd] = static_cast<uint32_t>(aibnd[iibnd]);
                boneWeights[iibnd] = agWeights[weightIndex];
            }
        }

        totalEdges.emplace_back(aposf[ipos], 1.0f);
        totalBoneIndices.push_back(boneIndices);
        totalBoneWeights.push_back(boneWeights);

        for (int ipose = 0; ipose < pglobset->cpose; ++ipose)
        {
            glm::vec3 dpos(0.0f);

            if (ipose < static_cast<int>(asubposef.size()) && ipos < asubposef[ipose].aiposf.size())
            {
                const uint16_t iposPose = asubposef[ipose].aiposf[ipos];

                if (iposPose < aposfPoses.size())
                    dpos = aposfPoses[iposPose] - aposf[ipos];
            }

            totalPoseDpos.emplace_back(dpos, 0.0f);
        }
    };

    for (int itwef = 0; itwef < ctwef; ++itwef)
    {
        const TWEF& twef = atwef[itwef];

        AppendCelPoint(twef.aipos1);
        AppendCelPoint(twef.aipos2);
        AppendCelPoint(twef.aipos0);
        AppendCelPoint(twef.aipos3);
    }
}

void BuildGlobsetSaaArray(GLOBSET* pglobset)
{
    pglobset->apsaa.resize(pglobset->cpsaa);

    for (int i = 0, saaIndex = 0; i < pglobset->cglob; i++)
    {
        SAA *psaa = pglobset->aglob[i].psaa;

        if (psaa != nullptr)
            pglobset->apsaa[saaIndex++] = psaa;
    }
}

void CloneGlob(GLOBSET* pglobset, GLOB* pglob, GLOBI* pglobi)
{
    if (!pglob || !pglobi)
        return;

    if (pglob->psaa)
        pglobset->cpsaa++;

    if (pglob->pwrbg)
    {
        // already created with correct palo/pglob during load
        // just keep this as safety if needed
        pglob->pwrbg->pglob = pglob;
    }

    if (pglob->pwrbg && pglob->pwrbg->cmat > 0)
    {
        if (!pglob->pwarpGlob)
            pglob->pwarpGlob = std::make_shared<WRBGLOB_GL>();

        WRBGLOB_GL& w = *pglob->pwarpGlob;

        w.basePos.clear();

        for (auto& subglob : pglob->asubglob)
        {
            for (auto& v : subglob.vertices)
                w.basePos.emplace_back(v.pos, 1.0f);
        }

        w.vertexCount = (int)w.basePos.size();

        if (w.ssboState == 0)
            glGenBuffers(1, &w.ssboState);
    }

    if (pglob->fThreeWay == 1 && pglob->fDynamic == 0 && pglob->pwarpGlob == nullptr)
    {
        uint32_t totalVerts = 0;

        for (auto& subglob : pglob->asubglob)
            totalVerts += (uint32_t)subglob.vertices.size();

        pglob->trlk = TRLK_Relight;

        glGenBuffers(1, &pglob->ssboCachedMaterial);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pglob->ssboCachedMaterial);
        glBufferData(GL_SHADER_STORAGE_BUFFER, totalVerts * sizeof(MATERIAL), nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    if (!pglob->asubglob.empty())
    {
        SetGlobDraw(pglob);
        numRo++;

        if (pglob->edgeCount > 0)
            numRoCel++;
    }
}

void CloneGlobset(GLOBSET* pglobset, ALO* palo, GLOBSET* pglobsetBase)
{
    *pglobset = *pglobsetBase;

    // Bone transforms are instance state.  Geometry buffers may be shared by
    // clones, but sharing this SSBO makes every clone overwrite the matrices
    // used by all of the others before the deferred render lists are drawn.
    // PostGlobsetLoad allocates and initializes a private buffer for the clone.
    pglobset->boneMatrixSSBO = 0;

    pglobset->cpsaa = 0;
    pglobset->pwrbgFirst.reset();

    for (int i = 0; i < pglobset->aglob.size(); i++)
    {
        GLOB& glob = pglobset->aglob[i];

        // Cached lighting is instance-dependent, so the clone must never
        // retain the source glob's OpenGL buffer handle.
        glob.ssboCachedMaterial = 0;

        // Deep-copy WRBG and repair owner pointers.
        if (glob.pwrbg)
        {
            glob.pwrbg = std::make_shared<WRBG>(*glob.pwrbg);

            glob.pwrbg->palo = palo;
            glob.pwrbg->pglob = &glob;

            glob.pwrbg->pwrbgNextGlobset = pglobset->pwrbgFirst;
            pglobset->pwrbgFirst = glob.pwrbg;
        }

        // Recount SAA array.
        if (glob.psaa)
            pglobset->cpsaa++;

        if (glob.fThreeWay == 1 && glob.fDynamic == 0 && glob.pwarpGlob == nullptr)
        {
            size_t totalVerts = 0;

            for (const SUBGLOB& subglob : glob.asubglob)
                totalVerts += subglob.vertices.size();

            glob.trlk = TRLK_Relight;

            glGenBuffers(1, &glob.ssboCachedMaterial);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.ssboCachedMaterial);
            glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(totalVerts * sizeof(MATERIAL)), nullptr, GL_STATIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }

        if (!glob.asubglob.empty())
        {
            SetGlobDraw(&glob);
            numRo++;

            if (glob.edgeCount > 0)
                numRoCel++;
        }
    }

    BuildGlobsetSaaArray(pglobset);
}

void PostGlobsetLoad(GLOBSET* pglobset, ALO* palo)
{
    for (int i = 0; i < pglobset->aglob.size(); i++)
    {
        GLOB* glob = &pglobset->aglob[i];

        // If this glob has an SAA, call its post-load hook.
        SAA *saa = glob->psaa;
        
        if (saa != nullptr)
            saa->pvtsaa->pfnPostSaaLoad(saa);
    }

    for (WRBG* wrbg = pglobset->pwrbgFirst.get(); wrbg; wrbg = wrbg->pwrbgNextGlobset.get())
    {
        WR* pwr = (WR*)PloFindSwObject(palo->psw, 0x104, wrbg->oid, palo);
        if (!pwr) continue;

        ApplyWrGlob(pwr, palo, wrbg->pglob);
    }

    if (pglobset->cbnd <= 0)
        return;

    if (pglobset->boneMatrices.size() != static_cast<size_t>(pglobset->cbnd))
        pglobset->boneMatrices.resize(pglobset->cbnd, glm::mat4(1.0f));

    if (pglobset->boneMatrixSSBO == 0)
        glGenBuffers(1, &pglobset->boneMatrixSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pglobset->boneMatrixSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, pglobset->boneMatrices.size() * sizeof(glm::mat4), pglobset->boneMatrices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void UpdateGlobset(GLOBSET* pglobset, ALO* palo, float dt)
{
    for (int i = 0; i < pglobset->apsaa.size(); i++)
    {
        SAA* saa = pglobset->apsaa[i];
        if (!saa)
            continue;

        if (!FUpdatableSaa(saa))
            continue;

        auto* vtsaa = saa->pvtscroller;
        if (!vtsaa || !vtsaa->pfnUpdateScroller)
            continue;

        vtsaa->pfnUpdateScroller((SCROLLER*)saa, dt);
    }

    // 2) Update WR matrices (driven by WRBG list)
    for (WRBG* wrbg = pglobset->pwrbgFirst.get(); wrbg != nullptr; wrbg = wrbg->pwrbgNextGlobset.get())
    {
        WR* pwr = wrbg->pwr;

        if (!pwr) continue;

        UpdateWrMatrixes(pwr);
    }
}

int  g_fogType = 1;
bool g_fRenderModels = true;
bool g_fRenderCollision = false;
bool g_fRenderCelBorders = true;
bool g_fBsp = true;
float g_uAlpha = 1.0;
SMP s_smpFade = { 2.0, 0.0, 0.1 };
SMP g_smpAlphaFade = { 2.0, 0.0, 0.1 };
glm::vec4 g_rgbaCel = glm::vec4(16.0f / 255.0f, 16.0f / 255.0f, 16.0f / 255.0f, 1.0);
