#include "ref.h"

#include <cstdio>
#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace
{
#if 0 // Re-enable after all levels are implemented to audit remaining TAGK_Err paths.
    const char* PchzTagk(TAGK tagk)
    {
        static constexpr const char* s_apchzTagk[] =
        {
            "None", "Set", "Define", "DefineMacro", "DefineMacroLambda",
            "Assert", "If", "Or", "And", "Cond", "Else", "Case", "Let",
            "While", "Lambda", "Begin", "Import", "Quote", "Dot", "Pipe",
            "S32", "F32", "Vector", "Matrix", "Clq", "Lm", "Smp", "Bool",
            "Symid", "Bifk", "Pair", "Proc", "Basic", "Method", "Void",
            "Err"
        };

        if (tagk == TAGK_Nil)
            return "Nil";

        const int index = static_cast<int>(tagk);
        if (index >= 0 && index < static_cast<int>(TAGK_Max))
            return s_apchzTagk[index];

        return "Invalid";
    }

    void BreakOnTagError(const CRef& ref, void* returnAddress)
    {
        std::fprintf(stderr,
            "[TAGK ERROR] ref=%p return=%p previousTag=%d(%s) "
            "n=%d g=%.9g bool=%d symid=%u bifk=%d basic=%p "
            "pair=%p proc=%p method=%p\n",
            static_cast<const void*>(&ref), returnAddress,
            static_cast<int>(ref.m_tagk), PchzTagk(ref.m_tagk),
            ref.m_n, ref.m_g, ref.m_bool, static_cast<unsigned>(ref.m_symid),
            static_cast<int>(ref.m_bifk), static_cast<void*>(ref.m_pbasic),
            static_cast<void*>(ref.m_ppair.get()),
            static_cast<void*>(ref.m_pproc.get()),
            static_cast<void*>(ref.m_pmethod.get()));
        std::fflush(stderr);

#if defined(_MSC_VER)
        __debugbreak();
#endif
    }
#endif

    void ClearOwnedPayloads(CRef& ref)
    {
        ref.m_ppair.reset();
        ref.m_pproc.reset();
        ref.m_pmethod.reset();
    }
}

void CRef::SetTag(TAGK tagk)
{
#if 0 // Re-enable after all levels are implemented to audit remaining TAGK_Err paths.
    if (tagk == TAGK_Err)
    {
#if defined(_MSC_VER)
        BreakOnTagError(*this, _ReturnAddress());
#else
        BreakOnTagError(*this, nullptr);
#endif
    }
#endif

    ClearOwnedPayloads(*this);
    m_tagk = tagk;
}

void CRef::SetS32(s32 n)
{
    ClearOwnedPayloads(*this);
    m_n = n;
    m_tagk = TAGK_S32;
}

void CRef::SetF32(f32 g)
{
    ClearOwnedPayloads(*this);
    m_g = g;
    m_tagk = TAGK_F32;
}

void CRef::SetBool(int fBool)
{
    ClearOwnedPayloads(*this);
    m_bool = fBool ? 1 : 0;
    m_n = m_bool;
    m_tagk = TAGK_Bool;
}

void CRef::SetSymid(SYMID symid)
{
    ClearOwnedPayloads(*this);
    m_symid = symid;
    m_tagk = TAGK_Symid;
}

void CRef::SetBifk(BIFK bifk)
{
    ClearOwnedPayloads(*this);
    m_bifk = bifk;
    m_tagk = TAGK_Bifk;
}

void CRef::SetPair(std::shared_ptr <CPair> ppair)
{
    ClearOwnedPayloads(*this);
    m_ppair = std::move(ppair);
    m_tagk = TAGK_Pair;
}

void CRef::SetProc(std::shared_ptr <CProc> pproc)
{
    ClearOwnedPayloads(*this);
    m_pproc = std::move(pproc);
    m_tagk = TAGK_Proc;
}

void CRef::SetVector(const glm::vec3& vector)
{
    ClearOwnedPayloads(*this);
    m_vector = vector;
    m_vector4 = glm::vec4(vector, 0.0f);
    m_tagk = TAGK_Vector;
}

void CRef::SetVector4(const glm::vec4& vector)
{
    ClearOwnedPayloads(*this);
    m_vector = glm::vec3(vector);
    m_vector4 = vector;
    m_tagk = TAGK_Vector;
}

void CRef::SetMatrix(const glm::mat4& matrix)
{
    ClearOwnedPayloads(*this);
    m_matrix = matrix;
    m_tagk = TAGK_Matrix;
}

void CRef::SetClq(const CLQ& clq)
{
    ClearOwnedPayloads(*this);
    m_clq = clq;
    m_tagk = TAGK_Clq;
}

void CRef::SetLm(const LM& lm)
{
    ClearOwnedPayloads(*this);
    m_lm = lm;
    m_tagk = TAGK_Lm;
}

void CRef::SetSmp(const SMP& smp)
{
    ClearOwnedPayloads(*this);
    m_smp = smp;
    m_tagk = TAGK_Smp;
}

void CRef::SetBasic(BASIC* pbasic)
{
    /*if (pbasic != nullptr && pbasic->pvtbasic == nullptr)
    {
        std::printf(
            "CRef::SetBasic ERROR: pbasic=%p has null pvtbasic\n",
            static_cast<void*>(pbasic));

        __debugbreak();
    }*/

    ClearOwnedPayloads(*this);
    m_pbasic = pbasic;
    m_tagk = TAGK_Basic;
}

void CRef::SetMethod(std::shared_ptr<CMethod> method)
{
    ClearOwnedPayloads(*this);
    m_pmethod = std::move(method);
    m_tagk = TAGK_Method;
}

SYMID CRef::GetSymid() const
{
    return m_symid;
}

bool CRef::operator==(const CRef& other) const
{
    if (m_tagk != other.m_tagk)
        return false;

    switch (m_tagk)
    {
        case TAGK_S32:
        return m_n == other.m_n;

        case TAGK_F32:
        return m_g == other.m_g;

        case TAGK_Bool:
        return m_bool == other.m_bool;

        case TAGK_Symid:
        return m_symid == other.m_symid;

        case TAGK_Bifk:
        return m_bifk == other.m_bifk;

        case TAGK_Pair:
        return m_ppair == other.m_ppair;

        case TAGK_Proc:
        return m_pproc == other.m_pproc;

        case TAGK_Vector:
        return m_vector == other.m_vector;

        case TAGK_Matrix:
        return m_matrix == other.m_matrix;

        case TAGK_Clq:
        return m_clq.g0 == other.m_clq.g0 &&
            m_clq.g1 == other.m_clq.g1 &&
            m_clq.g2 == other.m_clq.g2;

        case TAGK_Lm:
        return m_lm.gMin == other.m_lm.gMin &&
            m_lm.gMax == other.m_lm.gMax;

        case TAGK_Smp:
        return m_smp.svSlow == other.m_smp.svSlow &&
            m_smp.svFast == other.m_smp.svFast &&
            m_smp.dtFast == other.m_smp.dtFast;

        case TAGK_Basic:
        return m_pbasic == other.m_pbasic;

        case TAGK_Method:
        return m_pmethod == other.m_pmethod;

        default:
        return true;
    }
}

CRef CRef::RefCoerceS32() const
{
    CRef ret;

    if (m_tagk == TAGK_S32)
        ret = *this;
    else if (m_tagk == TAGK_F32)
        ret.SetS32(static_cast<s32>(m_g));
    else if (m_tagk == TAGK_Bool)
        ret.SetS32(m_bool);
    else
        ret.SetS32(m_n);

    return ret;
}

CRef CRef::RefCoerceF32() const
{
    CRef ret;

    if (m_tagk == TAGK_F32)
        ret = *this;
    else if (m_tagk == TAGK_S32)
        ret.SetF32(static_cast<f32>(m_n));
    else if (m_tagk == TAGK_Bool)
        ret.SetF32(static_cast<f32>(m_bool));
    else
        ret.SetF32(m_g);

    return ret;
}

void CRef::CloneTo(CRef* prefClone, CFrame* pframeClone)
{
    if (!prefClone)
        return;

    ClearOwnedPayloads(*prefClone);
    prefClone->m_tagk = m_tagk;

    switch (m_tagk)
    {
        case TAGK_Pair:
        {
            if (m_ppair)
            {
                auto pairClone = PpairNew();
                m_ppair->CloneTo(pairClone.get(), pframeClone);
                prefClone->m_ppair = pairClone;
            }
            else
                prefClone->m_ppair.reset();

            break;
        }

        case TAGK_Proc:
        {
            if (m_pproc)
            {
                auto procClone = PprocNew();
                m_pproc->CloneTo(procClone.get(), pframeClone);
                prefClone->m_pproc = procClone;
            }
            else
                prefClone->m_pproc.reset();

            break;
        }

        default:
        {
            *prefClone = *this;
            break;
        }
    }
}
