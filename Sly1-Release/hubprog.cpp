#include "hubprog.h"
#include "game.h"

HUBPROG* NewHubProg()
{
    return new HUBPROG{};
}

void CloneHubProg(HUBPROG* phubprog, HUBPROG* phubprogBase)
{
    CloneAlo(phubprog, phubprogBase);
}

void PostHubProgLoad(HUBPROG* phubprog)
{
    PostAloLoad(phubprog);
    SnipAloObjects(phubprog, 9, s_asnipHubProg);

    const int gameWorld = static_cast<int>(phubprog->oid) - 1300;

    if (static_cast<unsigned int>(phubprog->oid) - 1301U >= 5U)
        return;

    for (int i = 0; i < 9; ++i)
    {
        ALO* paloProgress = phubprog->apaloProgress[i];

        if (paloProgress == nullptr)
            continue;

        bool fComplete;

        if (i == 8)
            fComplete = (g_pgsCur->aws[gameWorld].fws & 0x20U) != 0;
        else
        {
            const int levelId = (gameWorld << 8) | i;
            const FLS tasks = TasksFromLevelID(levelId);
            const LS* pls = PlsFromWorldLevelKey(levelId);
            const int completionFlag = (tasks & FLS_KeyCollected) != 0
                ? FLS_KeyCollected
                : FLS_Visited;

            fComplete = pls != nullptr && (pls->grfls & completionFlag) != 0;
        }

        if (fComplete)
            paloProgress->pvtlo->pfnAddLo(paloProgress);
    }
}

void DeleteHubProg(HUBPROG* phubprog)
{
    delete phubprog;
}
