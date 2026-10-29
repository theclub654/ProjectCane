#include "glob.h"
#include "wr.h"
#include "render.h"
#include "loop.h"
#include "pingpong.h"
#include "shuffle.h"
#include "hologram.h"
#include "eyes.h"
#include "scroller.h"
#include "circler.h"
#include "looker.h"
#ifdef CANE_GL41
#include "runtime_error.h"
#include "glshaders.h"
#endif
#include <unordered_map>

namespace
{
    struct POSEWEIGHTGPU
    {
        GLuint buffer = 0;
        size_t count = 0;
        bool dirty = true;
    };

    std::unordered_map<const GLOBSET*, POSEWEIGHTGPU> s_poseWeightGpu;
}

static std::shared_ptr<SAA> CloneInstancedSaa(SAA* saaBase)
{
    std::shared_ptr<SAA> saa;

    switch (saaBase->saak)
    {
        case SAAK_Loop:     saa = std::make_shared<LOOP>(*static_cast<LOOP*>(saaBase)); break;
        case SAAK_PingPong: saa = std::make_shared<PINGPONG>(*static_cast<PINGPONG*>(saaBase)); break;
        case SAAK_Shuffle:  saa = std::make_shared<SHUFFLE>(*static_cast<SHUFFLE*>(saaBase)); break;
        case SAAK_Hologram: saa = std::make_shared<HOLOGRAM>(*static_cast<HOLOGRAM*>(saaBase)); break;
        case SAAK_Eyes:     saa = std::make_shared<EYES>(*static_cast<EYES*>(saaBase)); break;
        case SAAK_Scroller: saa = std::make_shared<SCROLLER>(*static_cast<SCROLLER*>(saaBase)); break;
        case SAAK_Circler:  saa = std::make_shared<CIRCLER>(*static_cast<CIRCLER*>(saaBase)); break;
        case SAAK_Looker:   saa = std::make_shared<LOOKER>(*static_cast<LOOKER*>(saaBase)); break;
        default: return {};
    }

    // An instance receives private animation state and must not inherit links
    // belonging to the source SAI update queue.
    saa->sai.psaiNext = nullptr;
    if (saa->saak == SAAK_Eyes)
        static_cast<EYES*>(saa.get())->saiOther.psaiNext = nullptr;

    g_apsaaSw.push_back(saa);
    return saa;
}

void LoadGlobsetFromBrx(GLOBSET* pglobset, ALO* palo, CBinaryInputStream* pbis)
{
    pglobset->cpsaa = 0;
    pglobset->boneMatrixSSBO = 0;
#ifdef CANE_GL41
    pglobset->boneMatrixTexture = 0;
#endif

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
            pglobset->aglob[i].psaa = PsaaLoadFromBrx(pbis).get();

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

                // PCBRX stores one fixed, interleaved vertex record. Read every
                // field explicitly so the file layout never depends on GLM padding.
                const uint16_t vertexCount = pbis->U16Read();
                const uint16_t triangleCount = pbis->U16Read();

                pglobset->aglob[i].asubglob[a].shdID = pbis->U16Read();
                pglobset->aglob[i].asubglob[a].cibnd = pbis->U16Read();
                pglobset->aglob[i].asubglob[a].pshd  = &g_ashd[pglobset->aglob[i].asubglob[a].shdID];

                pglobset->aglob[i].asubglob[a].unSelfIllum = pbis->F32Read();

                SUBGLOB& subglob = pglobset->aglob[i].asubglob[a];
                subglob.vertices.resize(vertexCount);
                for (VERTICE& vertex : subglob.vertices)
                {
                    vertex.pos = pbis->ReadVector();
                    vertex.normal = pbis->ReadVector();
                    vertex.color = pbis->ReadVector4();
                    vertex.uv = pbis->ReadVector2();
                    const uint16_t bone0 = pbis->U16Read();
                    const uint16_t bone1 = pbis->U16Read();
                    const uint16_t bone2 = pbis->U16Read();
                    const uint16_t bone3 = pbis->U16Read();
                    vertex.boneIndices = glm::uvec4(bone0, bone1, bone2, bone3);
                    vertex.boneWeights = pbis->ReadVector4();
                }

                subglob.indices.resize(triangleCount);
                for (INDICE& triangle : subglob.indices)
                {
                    triangle.v1 = pbis->U16Read();
                    triangle.v2 = pbis->U16Read();
                    triangle.v3 = pbis->U16Read();
                    if (triangle.v1 >= vertexCount || triangle.v2 >= vertexCount ||
                        triangle.v3 >= vertexCount)
                        throw std::runtime_error("PCBRX triangle index is out of range");
                }

                if (pglobset->cpose != 0)
                {
                    GLOB& glob = pglobset->aglob[i];
                    const size_t deltaCount = static_cast<size_t>(vertexCount) * pglobset->cpose;
                    glob.poseDpos.reserve(glob.poseDpos.size() + deltaCount);
                    glob.poseDnormal.reserve(glob.poseDnormal.size() + deltaCount);
                    for (size_t delta = 0; delta < deltaCount; ++delta)
                    {
                        glob.poseDpos.push_back(pbis->ReadVector4());
                        glob.poseDnormal.push_back(pbis->ReadVector4());
                    }
                    glob.poseVertexCount += static_cast<int>(vertexCount);
                }

                auto& glob = pglobset->aglob[i];
                auto* shd = glob.asubglob[a].pshd;

				// Background and cutout packets still use this compatibility flag
				// to select their sorted queues.  RP_Translucent is always sorted as
				// one retail bucket and must not be split by material blend state.
				if (!glob.fTransluscentSort && shd != nullptr && shd->grfshd == 2 &&
					(glob.rp == RP_Background || glob.rp == RP_Cutout ||
					 glob.rp == RP_CutoutAfterProjVolume))
				{
					glob.fTransluscentSort = 1;
				}

                if (glob.rp == RP_ProjVolume)
                    glob.grfshd = glob.asubglob[a].pshd->grfshd;

                if (shd->shdk == SHDK_ThreeWay)
                    glob.fThreeWay = 1;

                SAI* sai = nullptr;
                if (glob.psaa != nullptr)
                    sai = glob.psaa->pvtsaa->pfnPsaiFromSaaShd(glob.psaa, shd);
                if (!sai && shd->psaa)
                    sai = shd->psaa->pvtsaa->pfnPsaiFromSaaShd(shd->psaa, shd);

                subglob.uvSai = sai;
                subglob.usesUvAnim = (sai && (sai->grfsai & 0x2));
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
                std::vector <uint16_t> packedIdx(totalIndices);

                VERTICE*  vptr = packedVerts.data();
                uint16_t* iptr = packedIdx.data();

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
                        iptr[0] = tri.v1;
                        iptr[1] = tri.v2;
                        iptr[2] = tri.v3;
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
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(packedIdx.size() * sizeof(uint16_t)), packedIdx.data(), GL_STATIC_DRAW);

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
#ifdef CANE_GL41
                    // glob.vert reads these through RGBA32F buffer textures (no
                    // SSBOs on GL 4.1); instances share the names and the views.
                    glGenBuffers(1, &glob.poseDposSSBO);
                    glBindBuffer(GL_TEXTURE_BUFFER, glob.poseDposSSBO);
                    glBufferData(GL_TEXTURE_BUFFER, static_cast<GLsizeiptr>(glob.poseDpos.size() * sizeof(glm::vec4)), glob.poseDpos.data(), GL_STATIC_DRAW);
                    CreateBufferTexture(glob.poseDposSSBO, kPoseDposTextureFormat);

                    glGenBuffers(1, &glob.poseDnormalSSBO);
                    glBindBuffer(GL_TEXTURE_BUFFER, glob.poseDnormalSSBO);
                    glBufferData(GL_TEXTURE_BUFFER, static_cast<GLsizeiptr>(glob.poseDnormal.size() * sizeof(glm::vec4)), glob.poseDnormal.data(), GL_STATIC_DRAW);
                    CreateBufferTexture(glob.poseDnormalSSBO, kPoseDnormalTextureFormat);

                    glBindBuffer(GL_TEXTURE_BUFFER, 0);
#else
                    glGenBuffers(1, &glob.poseDposSSBO);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.poseDposSSBO);
                    glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(glob.poseDpos.size() * sizeof(glm::vec4)), glob.poseDpos.data(), GL_STATIC_DRAW);

                    glGenBuffers(1, &glob.poseDnormalSSBO);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.poseDnormalSSBO);
                    glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(glob.poseDnormal.size() * sizeof(glm::vec4)), glob.poseDnormal.data(), GL_STATIC_DRAW);

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif
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

#ifdef CANE_GL41
                        // Baked by transform feedback, read through a buffer texture.
                        glGenBuffers(1, &glob.ssboCachedMaterial);
                        glBindBuffer(GL_TEXTURE_BUFFER, glob.ssboCachedMaterial);
                        glBufferData(GL_TEXTURE_BUFFER, (GLsizeiptr)(totalVerts * sizeof(MATERIAL)), nullptr, GL_STATIC_DRAW);
                        CreateBufferTexture(glob.ssboCachedMaterial, kCachedMaterialTextureFormat);
                        glBindBuffer(GL_TEXTURE_BUFFER, 0);
#else
                        glGenBuffers(1, &glob.ssboCachedMaterial);
                        glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.ssboCachedMaterial);
                        glBufferData(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)(totalVerts * sizeof(MATERIAL)), nullptr, GL_STATIC_DRAW);
                        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif
                    }
                }

                SetGlobDraw(&pglobset->aglob[i]);
                numRo++;
            }

            pglobset->aglob[i].csubcel = pbis->U16Read();
            std::vector <CELPOINT> mergedEdgePoints;
            std::vector <glm::vec4> mergedEdgePoseDpos;

            for (int k = 0; k < pglobset->aglob[i].csubcel; k++)
            {
                const uint32_t edgeCount = pbis->U32Read();
                const size_t pointCount = static_cast<size_t>(edgeCount) * 4;
                glob.edgeCount += static_cast<GLsizei>(edgeCount);
                mergedEdgePoints.reserve(mergedEdgePoints.size() + pointCount);
                mergedEdgePoseDpos.reserve(mergedEdgePoseDpos.size() + pointCount * pglobset->cpose);

                for (size_t pointIndex = 0; pointIndex < pointCount; ++pointIndex)
                {
                    CELPOINT point{};
                    point.position = pbis->ReadVector4();
                    const uint16_t bone0 = pbis->U16Read();
                    const uint16_t bone1 = pbis->U16Read();
                    const uint16_t bone2 = pbis->U16Read();
                    const uint16_t bone3 = pbis->U16Read();
                    point.boneIndices = glm::uvec4(bone0, bone1, bone2, bone3);
                    point.boneWeights = pbis->ReadVector4();
                    mergedEdgePoints.push_back(point);
                }

                const size_t poseDeltaCount = pointCount * pglobset->cpose;

                for (size_t delta = 0; delta < poseDeltaCount; ++delta)
                    mergedEdgePoseDpos.push_back(pbis->ReadVector4());
            }

            if (glob.edgeCount > 0)
            {
                const size_t expectedEdgePointCount = static_cast<size_t>(glob.edgeCount) * 4;
                const size_t expectedEdgePoseCount = expectedEdgePointCount * glob.poseCount;

                assert(mergedEdgePoints.size() == expectedEdgePointCount);
                assert(mergedEdgePoseDpos.size() == expectedEdgePoseCount);

				glob.edgePoints = mergedEdgePoints;
				glob.edgePoseDpos = mergedEdgePoseDpos;

#ifdef CANE_GL41
                // celborder.vert reads these through buffer textures (no SSBOs on
                // GL 4.1): a CELPOINT is three RGBA32UI texels, a pose delta one
                // RGBA32F texel.
                static_assert(sizeof(CELPOINT) == 48, "celborder.vert reads a CELPOINT as three 16-byte texels");

                glGenBuffers(1, &glob.edgeSSBO);
                glBindBuffer(GL_TEXTURE_BUFFER, glob.edgeSSBO);
                glBufferData(GL_TEXTURE_BUFFER, static_cast<GLsizeiptr>(mergedEdgePoints.size() * sizeof(CELPOINT)), mergedEdgePoints.data(), GL_STATIC_DRAW);
                CreateBufferTexture(glob.edgeSSBO, kEdgeTextureFormat);

                if (!mergedEdgePoseDpos.empty())
                {
                    glGenBuffers(1, &glob.edgePoseDposSSBO);
                    glBindBuffer(GL_TEXTURE_BUFFER, glob.edgePoseDposSSBO);
                    glBufferData(GL_TEXTURE_BUFFER, static_cast<GLsizeiptr>(mergedEdgePoseDpos.size() * sizeof(glm::vec4)), mergedEdgePoseDpos.data(), GL_STATIC_DRAW);
                    CreateBufferTexture(glob.edgePoseDposSSBO, kEdgePoseDposTextureFormat);
                }

                glBindBuffer(GL_TEXTURE_BUFFER, 0);
#else
                glGenBuffers(1, &glob.edgeSSBO);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.edgeSSBO);
                glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(mergedEdgePoints.size() * sizeof(CELPOINT)), mergedEdgePoints.data(), GL_STATIC_DRAW);

                if (!mergedEdgePoseDpos.empty())
                {
                    glGenBuffers(1, &glob.edgePoseDposSSBO);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.edgePoseDposSSBO);
                    glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(mergedEdgePoseDpos.size() * sizeof(glm::vec4)), mergedEdgePoseDpos.data(), GL_STATIC_DRAW);
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif

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

            inst.edgeSSBO  = base.edgeSSBO;
            inst.edgeCount = base.edgeCount;
            inst.edgePoseDposSSBO = base.edgePoseDposSSBO;
			inst.edgePoints = base.edgePoints;
			inst.edgePoseDpos = base.edgePoseDpos;

            inst.poseDposSSBO = base.poseDposSSBO;
            inst.poseDnormalSSBO = base.poseDnormalSSBO;
            inst.poseCount = base.poseCount;
            inst.poseVertexCount = base.poseVertexCount;

            CloneGlob(pglobset, &inst, &pglobset->aglobi[i]);
        }

    }

    BuildGlobsetSaaArray(pglobset);
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
    {
        pglobset->cpsaa++;

        // Retail CloneGlob duplicates the SAA when it is marked instanced, or
        // when the glob has WR deformation state.  The clone must then point
        // its subglobs at the duplicated SAI instead of the source object's.
        if ((pglob->psaa->sai.grfsai & 0x04) != 0 || pglob->pwrbg != nullptr)
        {
            SAA* saaBase = pglob->psaa;
            std::shared_ptr<SAA> saaOwner = CloneInstancedSaa(saaBase);
            SAA* saa = saaOwner.get();

            if (saa != nullptr)
            {
                pglob->psaa = saa;

                for (SUBGLOB& subglob : pglob->asubglob)
                {
                    SAI* sai = saa->pvtsaa->pfnPsaiFromSaaShd(saa, subglob.pshd);
                    if (sai != nullptr)
                    {
                        subglob.uvSai = sai;
                        subglob.usesUvAnim = (sai->grfsai & 0x02) != 0;
                    }
                }
            }
        }
    }

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
#ifdef CANE_GL41
        glBindBuffer(GL_TEXTURE_BUFFER, pglob->ssboCachedMaterial);
        glBufferData(GL_TEXTURE_BUFFER, totalVerts * sizeof(MATERIAL), nullptr, GL_STATIC_DRAW);
        CreateBufferTexture(pglob->ssboCachedMaterial, kCachedMaterialTextureFormat);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
#else
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pglob->ssboCachedMaterial);
        glBufferData(GL_SHADER_STORAGE_BUFFER, totalVerts * sizeof(MATERIAL), nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif
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
#ifdef CANE_GL41
    pglobset->boneMatrixTexture = 0;
#endif
    // Pose-weight GPU state is kept in an external sidecar so cloning does
    // not copy an OpenGL handle or alter the reconstructed object layout.
    s_poseWeightGpu.erase(pglobset);

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

        // Retail CloneGlob gives every instanced shader animation private
        // state and redirects the cloned subglobs to that state.  The C++
        // vector copy above otherwise leaves clones sharing the base SAA,
        // so one object's LOOP update overwrites another object's requested
        // frame (notably the roulette alarm sensors).
        if (glob.psaa)
        {
            pglobset->cpsaa++;

            if ((glob.psaa->sai.grfsai & 0x04) != 0 || glob.pwrbg != nullptr)
            {
                std::shared_ptr<SAA> saaOwner = CloneInstancedSaa(glob.psaa);
                SAA* saa = saaOwner.get();

                if (saa != nullptr)
                {
                    glob.psaa = saa;

                    for (SUBGLOB& subglob : glob.asubglob)
                    {
                        SAI* sai = saa->pvtsaa->pfnPsaiFromSaaShd(saa, subglob.pshd);
                        if (sai != nullptr)
                        {
                            subglob.uvSai = sai;
                            subglob.usesUvAnim = (sai->grfsai & 0x02) != 0;
                        }
                    }
                }
            }
        }

        if (glob.fThreeWay == 1 && glob.fDynamic == 0 && glob.pwarpGlob == nullptr)
        {
            size_t totalVerts = 0;

            for (const SUBGLOB& subglob : glob.asubglob)
                totalVerts += subglob.vertices.size();

            glob.trlk = TRLK_Relight;

            glGenBuffers(1, &glob.ssboCachedMaterial);
#ifdef CANE_GL41
            glBindBuffer(GL_TEXTURE_BUFFER, glob.ssboCachedMaterial);
            glBufferData(GL_TEXTURE_BUFFER, static_cast<GLsizeiptr>(totalVerts * sizeof(MATERIAL)), nullptr, GL_STATIC_DRAW);
            CreateBufferTexture(glob.ssboCachedMaterial, kCachedMaterialTextureFormat);
            glBindBuffer(GL_TEXTURE_BUFFER, 0);
#else
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, glob.ssboCachedMaterial);
            glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(totalVerts * sizeof(MATERIAL)), nullptr, GL_STATIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif
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

    UploadGlobsetPoseWeights(pglobset);

    if (pglobset->cbnd <= 0)
        return;

    if (pglobset->boneMatrices.size() != static_cast<size_t>(pglobset->cbnd))
        pglobset->boneMatrices.resize(pglobset->cbnd, glm::mat4(1.0f));

#ifdef CANE_GL41
    // Each bone is four RGBA32F texels in the buffer texture dysh.vert,
    // celborder.vert and glob.vert read.
    static GLint s_maxTextureBufferTexels = -1;
    if (s_maxTextureBufferTexels < 0)
        glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &s_maxTextureBufferTexels);
    if (pglobset->cbnd * 4 > s_maxTextureBufferTexels)
    {
        ReportRuntimeError("Graphics", "Globset has %d bones (%d texels), above GL_MAX_TEXTURE_BUFFER_SIZE %d; dynamic shadows will skip the extra bones.",
            pglobset->cbnd, pglobset->cbnd * 4, s_maxTextureBufferTexels);
    }

    if (pglobset->boneMatrixSSBO == 0)
        glGenBuffers(1, &pglobset->boneMatrixSSBO);

    // Uploaded through GL_TEXTURE_BUFFER, which exists on GL 4.1 as well.
    glBindBuffer(GL_TEXTURE_BUFFER, pglobset->boneMatrixSSBO);
    glBufferData(GL_TEXTURE_BUFFER, pglobset->boneMatrices.size() * sizeof(glm::mat4), pglobset->boneMatrices.data(), GL_DYNAMIC_DRAW);

    if (pglobset->boneMatrixTexture == 0)
    {
        glGenTextures(1, &pglobset->boneMatrixTexture);
        glBindTexture(GL_TEXTURE_BUFFER, pglobset->boneMatrixTexture);
        glTexBuffer(GL_TEXTURE_BUFFER, kBoneMatrixTextureFormat, pglobset->boneMatrixSSBO);
        glBindTexture(GL_TEXTURE_BUFFER, 0);
    }

    glBindBuffer(GL_TEXTURE_BUFFER, 0);
#else
    if (pglobset->boneMatrixSSBO == 0)
        glGenBuffers(1, &pglobset->boneMatrixSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pglobset->boneMatrixSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, pglobset->boneMatrices.size() * sizeof(glm::mat4), pglobset->boneMatrices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif
}

void UploadGlobsetPoseWeights(GLOBSET* pglobset)
{
    if (pglobset == nullptr || pglobset->agPoses.empty())
        return;

    POSEWEIGHTGPU& gpu = s_poseWeightGpu[pglobset];

    if (gpu.buffer == 0 || gpu.count != pglobset->agPoses.size())
    {
        if (gpu.buffer == 0)
            glGenBuffers(1, &gpu.buffer);
#ifdef CANE_GL41
        // Uploaded through GL_TEXTURE_BUFFER (GL 4.1 has no SSBOs). celborder
        // and glob read it as an R32F buffer texture, kept in the
        // buffer-texture sidecar.
        glBindBuffer(GL_TEXTURE_BUFFER, gpu.buffer);
        glBufferData(GL_TEXTURE_BUFFER,
            static_cast<GLsizeiptr>(pglobset->agPoses.size() * sizeof(float)),
            pglobset->agPoses.data(), GL_DYNAMIC_DRAW);
        CreateBufferTexture(gpu.buffer, kPoseWeightTextureFormat);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
#else
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpu.buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
            static_cast<GLsizeiptr>(pglobset->agPoses.size() * sizeof(float)),
            pglobset->agPoses.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif
        gpu.count = pglobset->agPoses.size();
        gpu.dirty = false;
        return;
    }

    if (!gpu.dirty)
        return;

#ifdef CANE_GL41
    glBindBuffer(GL_TEXTURE_BUFFER, gpu.buffer);
    glBufferSubData(GL_TEXTURE_BUFFER, 0,
        static_cast<GLsizeiptr>(pglobset->agPoses.size() * sizeof(float)),
        pglobset->agPoses.data());
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
#else
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpu.buffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
        static_cast<GLsizeiptr>(pglobset->agPoses.size() * sizeof(float)),
        pglobset->agPoses.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif
    gpu.dirty = false;
}

void MarkGlobsetPoseWeightsDirty(GLOBSET* pglobset)
{
    if (pglobset != nullptr)
        s_poseWeightGpu[pglobset].dirty = true;
}

GLuint GetGlobsetPoseWeightBuffer(const GLOBSET* pglobset)
{
    const auto it = s_poseWeightGpu.find(pglobset);
    return it != s_poseWeightGpu.end() ? it->second.buffer : 0;
}

void DeleteGlobsetPoseWeightBuffer(GLOBSET* pglobset)
{
    const auto it = s_poseWeightGpu.find(pglobset);
    if (it == s_poseWeightGpu.end())
        return;

#ifdef CANE_GL41
    if (it->second.buffer != 0)
    {
        DeleteBufferTexture(it->second.buffer);
        glDeleteBuffers(1, &it->second.buffer);
    }
#else
    if (it->second.buffer != 0)
        glDeleteBuffers(1, &it->second.buffer);
#endif
    s_poseWeightGpu.erase(it);
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
bool g_fRenderDialogTriggers = false;
bool g_fRenderCelBorders = true;
bool g_fBsp = true;
float g_uAlpha = 1.0;
SMP s_smpFade = { 2.0, 0.0, 0.1 };
SMP g_smpAlphaFade = { 2.0, 0.0, 0.1 };
glm::vec4 g_rgbaCel = glm::vec4(16.0f / 255.0f, 16.0f / 255.0f, 16.0f / 255.0f, 1.0);
