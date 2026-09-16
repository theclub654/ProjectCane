#include "splicemap.h"
#include "eval.h"
#include "frame.h"

void LoadSwSpliceFromBrx(SW* psw, CBinaryInputStream* pbis)
{
    CRef refEmptyList;
    refEmptyList.SetTag(TAGK_None);

    const uint32_t spliceCount = pbis->U32Read();
    const uint32_t symidMax = pbis->U32Read();
    const uint32_t extraOptCount = pbis->U32Read();

    psw->csplcSplice = spliceCount;
    psw->asplcSplice.clear();
    psw->asplcSplice.resize(spliceCount);

    for (uint32_t i = 0; i < spliceCount; i++)
    {
        SPLC &splc = psw->asplcSplice[i];

        splc.ppairCompile = PpairSerializeIn(pbis);
        splc.pframe = nullptr;

        if (g_fLoadDebugInfo)
        {
            splc.achzObjectPath = pbis->ReadStringSw();
            splc.achzFile = pbis->ReadStringSw();
            splc.achzSpliceSource = pbis->ReadStringSw();
        }
        else
        {
            char buffer[64];
            std::snprintf(buffer, sizeof(buffer), "ISPLICE_%#x", i);

            std::string name = buffer;

            splc.achzObjectPath = name;
            splc.achzFile = name;
            splc.achzSpliceSource = name;
        }
    }

    psw->symidMax = symidMax;
    psw->coptidExtra = extraOptCount;

    psw->mpsymidachz.resize(symidMax);
    psw->mpoptidachzExtra.resize(extraOptCount);

    if (g_fLoadDebugInfo)
    {
        for (uint32_t i = 0; i < symidMax; i++)
            psw->mpsymidachz[i] = pbis->ReadStringSw();

        for (uint32_t i = 0; i < extraOptCount; i++)
            psw->mpoptidachzExtra[i] = pbis->ReadStringSw();
    }
    else
    {
        for (uint32_t i = 0; i < symidMax; i++)
        {
            char buffer[64];
            std::snprintf(buffer, sizeof(buffer), "SYMID_%#x", i);
            psw->mpsymidachz[i] = buffer;
        }

        for (uint32_t i = 0; i < extraOptCount; i++)
        {
            char buffer[64];
            std::snprintf(buffer, sizeof(buffer), "OPTID_%#x", i);
            psw->mpoptidachzExtra[i] = buffer;
        }
    }
    
    psw->pframe = PframeNew();
    g_gc.AddRootFrame(psw->pframe.get());

    psw->symidScheduledCallbackList = static_cast<SYMID>(symidMax);
    psw->pframe->RefAddBinding(psw->symidScheduledCallbackList, &refEmptyList);

    for (uint32_t isplc = 0; isplc < spliceCount; ++isplc)
    {
        RefEvalModule(psw, isplc);
    }
}

std::shared_ptr <CFrame> PframeFromIsplice(int isplice, SW* psw)
{
    if (!psw || isplice < 0 || isplice >= psw->csplcSplice)
        return nullptr;

    auto frame = psw->asplcSplice[isplice].pframe;

    if (frame)
        g_gc.AddRootFrame(frame.get());

    return frame;
}
