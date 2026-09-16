#include "serialize.h"
std::shared_ptr <CPair> PpairSerializeIn(CBinaryInputStream* pbis)
{
    uint8_t tagByte = pbis->U8Read();

    if (tagByte == 0xFF)
        return nullptr;

    auto pair = std::make_shared<CPair>();

    TAGK tag = static_cast<TAGK>(tagByte & 0x7F);
    bool hasNext = (tagByte & 0x80) != 0;

    if (g_fLoadDebugInfo)
    {
        pbis->U16Read();
        pbis->U16Read();
        pbis->U16Read();
        pbis->U16Read();
        pbis->U16Read();
    }

    switch (tag)
    {
        case TAGK_S32:
        pair->m_ref.SetS32(pbis->S32Read());
        break;

        case TAGK_F32:
        pair->m_ref.SetF32(pbis->F32Read());
        break;

        case TAGK_Vector:
        {
            glm::vec3 v{};
            v.x = pbis->F32Read();
            v.y = pbis->F32Read();
            v.z = pbis->F32Read();
            pair->m_ref.SetVector(v);
            break;
        }

        case TAGK_Matrix:
        {
            glm::mat4 m{ 1.0f };
            float* p = glm::value_ptr(m);

            for (int i = 0; i < 16; i++)
                p[i] = pbis->F32Read();

            pair->m_ref.SetMatrix(m);
            break;
        }

        case TAGK_Clq:
        {
            CLQ clq{};
            clq.g0 = pbis->F32Read();
            clq.g1 = pbis->F32Read();
            clq.g2 = pbis->F32Read();
            pair->m_ref.SetClq(clq);
            break;
        }

        case TAGK_Lm:
        {
            LM lm{};
            lm.gMin = pbis->F32Read();
            lm.gMax = pbis->F32Read();
            pair->m_ref.SetLm(lm);
            break;
        }

        case TAGK_Smp:
        {
            SMP smp{};
            smp.svSlow = pbis->F32Read();
            smp.svFast = pbis->F32Read();
            smp.dtFast = pbis->F32Read();
            pair->m_ref.SetSmp(smp);
            break;
        }

        case TAGK_Bool:
        pair->m_ref.SetBool(pbis->U32Read() != 0);
        break;

        case TAGK_Symid:
        pair->m_ref.SetSymid(static_cast<SYMID>(pbis->U32Read()));
        break;

        case TAGK_Bifk:
        pair->m_ref.SetBifk(static_cast<BIFK>(pbis->U32Read()));
        break;

        case TAGK_Pair:
        pair->m_ref.SetPair(PpairSerializeIn(pbis));
        break;

        case TAGK_Basic:
        // If BASIC is really an ID, change CRef to store that ID instead.
        pair->m_ref.SetBasic((BASIC*)pbis->U32Read());
        //std::cout<< pbis->U32Read() <<"\n";
        break;

        default:
        pair->m_ref.SetTag(tag);
        break;
    }

    if (hasNext)
        pair->m_ppairNext = PpairSerializeIn(pbis);

    return pair;
}

int g_fLoadDebugInfo = 0;
