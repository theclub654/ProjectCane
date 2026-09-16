#include "clip.h"

int ClsgClipEdgeToObjectPruned(SO* pso, BSP* pbspPruned, glm::vec3* ppos0, glm::vec3* ppos1, int clsgMax, LSG* alsg)
{
    if (pbspPruned != nullptr) {
        return ClsgClipEdgeToBsp(pbspPruned, ppos0, ppos1, nullptr, clsgMax, alsg);
    }

    if (!pso->fSphere) {
        return 0;
    }

    return ClsgClipEdgeToSphere(&pso->xf.posWorld, pso->sRadiusSelf, ppos0, ppos1, clsgMax, alsg);
}