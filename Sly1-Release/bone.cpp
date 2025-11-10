#include "bone.h"

BONE* NewBone()
{
    return new BONE{};
}

int GetBoneSize()
{
    return sizeof(BONE);
}

void CloneBone(BONE* pbone, BONE* pboneBase)
{
    CloneSo(pbone, pboneBase);
}

void RenderBoneSelf(BONE* pbone, CM* pcm, RO* pro)
{
    RenderSoSelf(pbone, pcm, pro);
    RenderAloAsBone(pbone, pcm, pro);
}

void DeleteBone(BONE* pbone)
{
    delete pbone;
}

LBONE* NewLBone()
{
    return new LBONE{};
}

int GetLBoneSize()
{
    return sizeof(LBONE);
}

void CloneLBone(LBONE* plbone, LBONE* plboneBase)
{
    CloneAlo(plbone, plboneBase);
}

void RenderLboneSelf(LBONE* plbone, CM* pcm, RO* pro)
{
    RenderAloSelf(plbone, pcm, pro);
    RenderAloAsBone(plbone, pcm, pro);
}

void RenderAloAsBone(ALO* palo, CM* pcm, RO* pro)
{
    if (!g_fShowBones) return;

    ALO* lineRenderer = static_cast<ALO*>(palo->psw->aploStock[0xc]);
    if (!lineRenderer) return;

    glm::vec3 posBase = pro ? glm::vec3(pro->model[3]) : palo->xf.posWorld;

    bool renderedAny = false;
    for (ALO* child = palo->dlChild.paloFirst; child; child = child->dleChild.paloNext) {
        if ((child->pvtlo->grfcid & 1U) == 0) continue;

        glm::vec3 posChild;
        if (pro) {
            // Transform child's local offset using RO matrix
            glm::vec4 localPos = glm::vec4(child->xf.posWorld, 1.0);
            posChild = glm::vec3(pro->model * glm::vec4(localPos));
        }
        else {
            // Raw packed child world pos from legacy ALO layout
            posChild.x = child->xf.posWorld.x;
            posChild.y = child->xf.posWorld.y;
            posChild.z = child->xf.posWorld.z;
        }

        RenderAloLine(lineRenderer, pcm, &posBase, &posChild, 1.0f, 1.0f);
        renderedAny = true;
    }

    if (!renderedAny) {
        glm::vec3 posChild;
        if (pro) {
            posChild = glm::vec3(pro->model * glm::vec4(s_posDefaultBone, 1.0f));
        }
        else {
            ConvertAloPos(palo, nullptr, s_posDefaultBone, posChild);
        }

        RenderAloLine(lineRenderer, pcm, &posBase, &posChild, 1.0f, 1.0f);
    }

    /*if (g_fShowAxes) {
        RO ro;
        DupAloRo(palo, pro, &ro);
        glm::vec3 posWorld = glm::vec3(ro.mat[3]);
        DrawAxesWorld(&posWorld, reinterpret_cast<glm::mat3*>(&ro), 10.0f, pcm, 0);
    }*/
}

void DeleteLBone(LBONE* plbone)
{
    delete plbone;
}

int g_fShowBones = 0;
glm::vec3 s_posDefaultBone = glm::vec3(25.0, 0.0, 0.0);