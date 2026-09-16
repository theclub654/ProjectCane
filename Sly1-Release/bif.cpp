#include "bif.h"
#include "so.h"
#include "aseg.h"
#include "spliceobj.h"
#include <set>
#include <cstdio>
#include <cmath>

namespace
{
    bool IsInteger(const CRef& ref)
    {
        return ref.m_tagk == TAGK_S32;
    }

    bool IsFloat(const CRef& ref)
    {
        return ref.m_tagk == TAGK_F32;
    }

    bool IsNumber(const CRef& ref)
    {
        return IsInteger(ref) || IsFloat(ref);
    }

    CRef MakeNumber(float value, bool forceFloat)
    {
        CRef ret;

        if (forceFloat)
            ret.SetF32(value);
        else
            ret.SetS32(static_cast<s32>(value));

        return ret;
    }

    CRef CompareNumbers(int carg, CRef* aref, bool (*cmp)(float, float))
    {
        if (!aref || carg < 2)
            return MakeBoolRef(false);

        for (int i = 1; i < carg; ++i)
        {
            if (!IsNumber(aref[i - 1]) || !IsNumber(aref[i]))
                return MakeBoolRef(false);

            if (!cmp(RefAsF32(aref[i - 1]), RefAsF32(aref[i])))
                return MakeBoolRef(false);
        }

        return MakeBoolRef(true);
    }

    bool RefDeepEqualInternal(const CRef& a, const CRef& b, std::set<std::pair<const CPair*, const CPair*>>& comparedPairs);

    bool PairDeepEqual(const CPair* a, const CPair* b, std::set<std::pair<const CPair*, const CPair*>>& comparedPairs)
    {
        if (a == b)
            return true;
        if (!a || !b)
            return false;

        const auto key = std::make_pair(a, b);
        if (!comparedPairs.insert(key).second)
            return true;

        return RefDeepEqualInternal(a->m_ref, b->m_ref, comparedPairs) &&
            PairDeepEqual(a->m_ppairNext.get(), b->m_ppairNext.get(), comparedPairs);
    }

    bool RefDeepEqualInternal(
        const CRef& a,
        const CRef& b,
        std::set<std::pair<const CPair*, const CPair*>>& comparedPairs)
    {
        if (a.m_tagk != b.m_tagk)
            return false;

        switch (a.m_tagk)
        {
            case TAGK_None:
            case TAGK_Set:
            case TAGK_Define:
            case TAGK_DefineMacro:
            case TAGK_DefineMacroLambda:
            case TAGK_Assert:
            case TAGK_If:
            case TAGK_Or:
            case TAGK_And:
            case TAGK_Cond:
            case TAGK_Else:
            case TAGK_Case:
            case TAGK_Let:
            case TAGK_While:
            case TAGK_Lambda:
            case TAGK_Begin:
            case TAGK_Import:
            case TAGK_Quote:
            case TAGK_Dot:
            case TAGK_Pipe:
            case TAGK_Void:
            return true;

            case TAGK_S32:
            return a.m_n == b.m_n;

            case TAGK_Bool:
            return a.m_bool == b.m_bool;

            case TAGK_Symid:
            return a.m_symid == b.m_symid;

            case TAGK_Bifk:
            return a.m_bifk == b.m_bifk;

            case TAGK_Basic:
            return a.m_pbasic == b.m_pbasic;

            case TAGK_Pair:
            return PairDeepEqual(a.m_ppair.get(), b.m_ppair.get(), comparedPairs);

            case TAGK_Proc:
            {
                if (a.m_pproc == b.m_pproc)
                    return true;
                if (!a.m_pproc || !b.m_pproc)
                    return false;

                return a.m_pproc->m_crefReq == b.m_pproc->m_crefReq &&
                    a.m_pproc->m_fVarArg == b.m_pproc->m_fVarArg &&
                    a.m_pproc->m_pframe == b.m_pproc->m_pframe &&
                    PairDeepEqual(
                        a.m_pproc->m_ppairCodeExpr,
                        b.m_pproc->m_ppairCodeExpr,
                        comparedPairs);
            }

            case TAGK_Method:
            {
                if (a.m_pmethod == b.m_pmethod)
                    return true;
                if (!a.m_pmethod || !b.m_pmethod)
                    return false;

                return a.m_pmethod->m_pbasic == b.m_pmethod->m_pbasic &&
                    a.m_pmethod->m_pfnthunk == b.m_pmethod->m_pfnthunk &&
                    a.m_pmethod->m_crefReq == b.m_pmethod->m_crefReq;
            }

            // The original RefEqualHelper has no value-comparison cases for
            // F32, Vector, Matrix, Clq, Lm, or Smp.  Those types are compared by
            // their dedicated numeric/vector operations instead.
            default:
            return false;
        }
    }

    bool RefDeepEqual(const CRef& a, const CRef& b)
    {
        std::set<std::pair<const CPair*, const CPair*>> comparedPairs;
        return RefDeepEqualInternal(a, b, comparedPairs);
    }

    static CRef MakeVoidRef()
    {
        CRef ret;
        ret.SetTag(TAGK_Void);
        return ret;
    }

    static CRef MakeNoneRef()
    {
        CRef ret;
        ret.SetTag(TAGK_None);
        return ret;
    }

    static CRef MakeBoolRef(bool value)
    {
        CRef ret;
        ret.SetBool(value ? 1 : 0);
        return ret;
    }

    static bool IsFalse(const CRef& ref)
    {
        return ref.m_tagk == TAGK_Bool && ref.m_bool == 0;
    }

    static bool IsTruthy(const CRef& ref)
    {
        return !IsFalse(ref);
    }

    static bool FloatsNear(float a, float b, float epsilon)
    {
        return std::fabs(a - b) <= epsilon;
    }

    static std::shared_ptr<CPair> CopyList(std::shared_ptr<CPair> head)
    {
        if (!head)
            return nullptr;

        std::shared_ptr<CPair> newHead;
        std::shared_ptr<CPair> tail;

        for (auto node = head; node; node = node->m_ppairNext)
        {
            auto copy = MakePairNode(node->m_ref);

            if (!newHead)
                newHead = copy;
            else
                tail->m_ppairNext = copy;

            tail = copy;
        }

        return newHead;
    }

    static CRef MakePairRef(std::shared_ptr<CPair> pair)
    {
        CRef ret;
        if (pair)
            ret.SetPair(std::move(pair));
        else
            ret.SetTag(TAGK_None);
        return ret;
    }

    static bool IsListLike(const CRef& ref)
    {
        return ref.m_tagk == TAGK_Pair || ref.m_tagk == TAGK_None || ref.m_tagk == TAGK_Nil;
    }

    static void AppendNode(std::shared_ptr<CPair>& head, std::shared_ptr<CPair>& tail, const std::shared_ptr<CPair>& node)
    {
        if (!node)
            return;

        if (!head)
            head = node;
        else
            tail->m_ppairNext = node;

        tail = node;
    }

    static std::shared_ptr<CPair> MakeQuoteExpr(const CRef& value)
    {
        auto quoteHead = PpairNew();
        quoteHead->m_ref.SetTag(TAGK_Quote);

        auto quotedValue = MakePairNode(value);
        quoteHead->m_ppairNext = quotedValue;

        auto wrapper = PpairNew();
        wrapper->m_ref.SetPair(quoteHead);
        return wrapper;
    }

    static CRef EvalCall2(const CRef& callable, const CRef& arg, CFrame* pframe)
    {
        auto opNode = MakePairNode(callable);
        opNode->m_ppairNext = MakeQuoteExpr(arg);

        auto wrapper = PpairNew();
        wrapper->m_ref.SetPair(opNode);

        return RefEval(wrapper.get(), pframe);
    }

    static float RandomFloat01()
    {
        return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    }

    // =====================================================
    // Local helpers
    // =====================================================

    static float RefAsF32(const CRef& ref)
    {
        switch (ref.m_tagk)
        {
            case TAGK_S32:
            return static_cast<float>(ref.m_n);

            case TAGK_F32:
            return ref.m_g;

            case TAGK_Bool:
            return ref.m_bool ? 1.0f : 0.0f;

            default:
            return 0.0f;
        }
    }

    static int RefAsS32(const CRef& ref)
    {
        switch (ref.m_tagk)
        {
            case TAGK_S32:
            return ref.m_n;

            case TAGK_F32:
            return static_cast<int>(ref.m_g);

            case TAGK_Bool:
            return ref.m_bool ? 1 : 0;

            default:
            return 0;
        }
    }

    static glm::vec3 RefAsVector(const CRef& ref)
    {
        if (ref.m_tagk == TAGK_Vector)
            return ref.m_vector;

        return glm::vec3{};
    }

    static glm::mat4 RefAsMatrix(const CRef& ref)
    {
        if (ref.m_tagk == TAGK_Matrix)
            return ref.m_matrix;

        return glm::mat4{ 1.0f };
    }

    static bool HasArg(int argc, int index)
    {
        return index >= 0 && index < argc;
    }

    static std::shared_ptr<CPair> MakePairNode(const CRef& ref)
    {
        auto node = PpairNew();
        node->m_ref = ref;
        return node;
    }

    static int RefAsOptid(const CRef& ref)
    {
        return RefAsS32(ref);
    }

    static CLQ RefAsClq(const CRef& ref)
    {
        if (ref.m_tagk == TAGK_Clq)
            return ref.m_clq;

        return CLQ{};
    }

    static LM RefAsLm(const CRef& ref)
    {
        if (ref.m_tagk == TAGK_Lm)
            return ref.m_lm;

        return LM{};
    }

    static BASIC* RefAsBasicRaw(const CRef& ref)
    {
        if (ref.m_tagk == TAGK_Basic && ref.m_pbasic)
            return ref.m_pbasic;

        return nullptr;
    }

    static LO* RefAsLo(const CRef& ref)
    {
        return static_cast <LO*>(RefAsBasicRaw(ref));
    }

    static ALO* RefAsAlo(const CRef& ref)
    {
        return static_cast <ALO*>(RefAsBasicRaw(ref));
    }

    static SO* RefAsSo(const CRef& ref)
    {
        return static_cast <SO*>(RefAsBasicRaw(ref));
    }

    static ASEG* RefAsAseg(const CRef& ref)
    {
        return static_cast <ASEG*>(RefAsBasicRaw(ref));
    }

    static AMB* RefAsAmb(const CRef& ref)
    {
        return reinterpret_cast <AMB*>(static_cast<intptr_t>(RefAsS32(ref)));
    }

    static std::shared_ptr<BASIC> BasicSharedFromRaw(BASIC* ptr)
    {
        // IMPORTANT:
        // This is non-owning. It prevents shared_ptr from deleting engine-owned objects.
        return std::shared_ptr<BASIC>(ptr, [](BASIC*) {});
    }

    static CRef RefFromBasic(BASIC* basic)
    {
        CRef ret;
        ret.SetBasic(basic);
        return ret;
    }

    static std::shared_ptr<CPair> PairListFromObjects(LO** objects, int count)
    {
        std::shared_ptr<CPair> head;
        std::shared_ptr<CPair> tail;

        for (int i = 0; i < count; ++i)
        {
            if (!objects[i])
                continue;

            auto node = PpairNew();
            node->m_ref = RefFromBasic(reinterpret_cast<BASIC*>(objects[i]));

            if (!head)
                head = node;
            else
                tail->m_ppairNext = node;

            tail = node;
        }

        return head;
    }

    static std::shared_ptr<CPair> PairListFromSoVector(const std::vector<SO*>& objects)
    {
        std::shared_ptr<CPair> head;
        std::shared_ptr<CPair> tail;

        for (SO* so : objects)
        {
            if (!so)
                continue;

            auto node = PpairNew();
            node->m_ref = RefFromBasic(reinterpret_cast<BASIC*>(so));

            if (!head)
                head = node;
            else
                tail->m_ppairNext = node;

            tail = node;
        }

        return head;
    }

    static CRef RefFromPairList(std::shared_ptr<CPair> list)
    {
        CRef ret;

        if (list)
            ret.SetPair(list);
        else
            ret.SetTag(TAGK_None);

        return ret;
    }

}

CRef RefOpAdd(int carg, CRef* aref, CFrame*)
{
    if (!aref || carg <= 0)
        return MakeNumber(0.0f, false);

    switch (aref[0].m_tagk)
    {
        case TAGK_Vector:
        {
            glm::vec3 out = aref[0].m_vector;
            for (int i = 1; i < carg; ++i)
            {
                if (aref[i].m_tagk == TAGK_Vector)
                    out += aref[i].m_vector;
            }

            CRef ret;
            ret.SetVector(out);
            return ret;
        }

        case TAGK_Matrix:
        {
            glm::mat4 out = aref[0].m_matrix;
            for (int i = 1; i < carg; ++i)
            {
                if (aref[i].m_tagk == TAGK_Matrix)
                    out += aref[i].m_matrix;
            }

            CRef ret;
            ret.SetMatrix(out);
            return ret;
        }

        default:
        break;
    }

    bool forceFloat = false;
    float total = 0.0f;

    for (int i = 0; i < carg; ++i)
    {
        if (!IsNumber(aref[i]))
            continue;

        forceFloat = forceFloat || IsFloat(aref[i]);
        total += RefAsF32(aref[i]);
    }

    return MakeNumber(total, forceFloat);
}

CRef RefOpSub(int carg, CRef* aref, CFrame*)
{
    if (!aref || carg <= 0)
        return MakeNumber(0.0f, false);

    switch (aref[0].m_tagk)
    {
        case TAGK_Vector:
        {
            glm::vec3 out = aref[0].m_vector;
            for (int i = 1; i < carg; ++i)
            {
                if (aref[i].m_tagk == TAGK_Vector)
                    out -= aref[i].m_vector;
            }

            CRef ret;
            ret.SetVector(out);
            return ret;
        }

        case TAGK_Matrix:
        {
            glm::mat4 out = aref[0].m_matrix;
            for (int i = 1; i < carg; ++i)
            {
                if (aref[i].m_tagk == TAGK_Matrix)
                    out -= aref[i].m_matrix;
            }

            CRef ret;
            ret.SetMatrix(out);
            return ret;
        }

        default:
        break;
    }

    bool forceFloat = IsFloat(aref[0]);
    float total = RefAsF32(aref[0]);

    if (carg > 1)
    {
        for (int i = 1; i < carg; ++i)
        {
            if (!IsNumber(aref[i]))
                continue;

            forceFloat = forceFloat || IsFloat(aref[i]);
            total -= RefAsF32(aref[i]);
        }
    }

    return MakeNumber(total, forceFloat);
}

CRef RefOpMult(int carg, CRef* aref, CFrame*)
{
    if (!aref || carg <= 0)
        return MakeNumber(1.0f, false);

    CRef ret;

    // The original accumulator can begin as a scalar and promote when a
    // compound operand is encountered. Preserve that operand-order behavior.
    int complexIndex = -1;
    float prefixScale = 1.0f;
    for (int i = 0; i < carg; ++i)
    {
        if (IsNumber(aref[i]))
        {
            prefixScale *= RefAsF32(aref[i]);
            continue;
        }

        if (aref[i].m_tagk == TAGK_Vector ||
            aref[i].m_tagk == TAGK_Matrix ||
            aref[i].m_tagk == TAGK_Clq ||
            aref[i].m_tagk == TAGK_Lm)
        {
            complexIndex = i;
        }
        break;
    }

    if (complexIndex > 0)
    {
        std::vector<CRef> promoted;
        promoted.reserve(static_cast<size_t>(carg - complexIndex + 1));
        promoted.push_back(aref[complexIndex]);

        CRef scale;
        scale.SetF32(prefixScale);
        promoted.push_back(scale);

        for (int i = complexIndex + 1; i < carg; ++i)
            promoted.push_back(aref[i]);

        return RefOpMult(static_cast<int>(promoted.size()), promoted.data(), nullptr);
    }

    if (aref[0].m_tagk == TAGK_Vector)
    {
        glm::vec3 out = aref[0].m_vector;
        for (int i = 1; i < carg; ++i)
        {
            if (IsNumber(aref[i]))
                out *= RefAsF32(aref[i]);
            else if (aref[i].m_tagk == TAGK_Vector)
                out *= aref[i].m_vector;
        }

        ret.SetVector(out);
        return ret;
    }

    if (aref[0].m_tagk == TAGK_Matrix)
    {
        glm::mat4 out = aref[0].m_matrix;
        for (int i = 1; i < carg; ++i)
        {
            if (aref[i].m_tagk == TAGK_Matrix)
                out *= aref[i].m_matrix;
            else if (aref[i].m_tagk == TAGK_Vector)
            {
                // Splice matrix/vector multiplication is a direction transform:
                // use the matrix's rotational 3x3 portion and return a vector.
                // In particular, the tank script uses this to turn (1500, 0, 0)
                // into the RWM launch velocity.
                glm::vec3 vectorOut = glm::mat3(out) * aref[i].m_vector;
                for (++i; i < carg; ++i)
                {
                    if (IsNumber(aref[i]))
                        vectorOut *= RefAsF32(aref[i]);
                    else if (aref[i].m_tagk == TAGK_Vector)
                        vectorOut *= aref[i].m_vector;
                }

                ret.SetVector(vectorOut);
                return ret;
            }
            else if (IsNumber(aref[i]))
                out *= RefAsF32(aref[i]);
        }

        ret.SetMatrix(out);
        return ret;
    }

    if (aref[0].m_tagk == TAGK_Clq)
    {
        CLQ out = aref[0].m_clq;
        for (int i = 1; i < carg; ++i)
        {
            if (!IsNumber(aref[i]))
                continue;

            const float s = RefAsF32(aref[i]);
            out.g0 *= s;
            out.g1 *= s;
            out.g2 *= s;
        }

        ret.SetClq(out);
        return ret;
    }

    if (aref[0].m_tagk == TAGK_Lm)
    {
        LM out = aref[0].m_lm;
        for (int i = 1; i < carg; ++i)
        {
            if (!IsNumber(aref[i]))
                continue;

            const float s = RefAsF32(aref[i]);
            out.gMin *= s;
            out.gMax *= s;
        }

        ret.SetLm(out);
        return ret;
    }

    bool forceFloat = false;
    float total = 1.0f;

    for (int i = 0; i < carg; ++i)
    {
        if (!IsNumber(aref[i]))
            continue;

        forceFloat = forceFloat || IsFloat(aref[i]);
        total *= RefAsF32(aref[i]);
    }

    return MakeNumber(total, forceFloat);
}

CRef RefOpDiv(int carg, CRef* aref, CFrame*)
{
    if (!aref || carg <= 0)
        return MakeNumber(0.0f, true);

    CRef ret;

    if (aref[0].m_tagk == TAGK_Vector)
    {
        glm::vec3 out = aref[0].m_vector;
        for (int i = 1; i < carg; ++i)
        {
            if (IsNumber(aref[i]))
                out /= RefAsF32(aref[i]);
        }

        ret.SetVector(out);
        return ret;
    }

    if (aref[0].m_tagk == TAGK_Matrix)
    {
        glm::mat4 out = aref[0].m_matrix;
        for (int i = 1; i < carg; ++i)
        {
            if (IsNumber(aref[i]))
                out *= 1.0f / RefAsF32(aref[i]);
        }

        ret.SetMatrix(out);
        return ret;
    }

    if (aref[0].m_tagk == TAGK_Clq)
    {
        CLQ out = aref[0].m_clq;
        for (int i = 1; i < carg; ++i)
        {
            if (!IsNumber(aref[i]))
                continue;

            const float inverse = 1.0f / RefAsF32(aref[i]);
            out.g0 *= inverse;
            out.g1 *= inverse;
            out.g2 *= inverse;
        }

        ret.SetClq(out);
        return ret;
    }

    if (aref[0].m_tagk == TAGK_Lm)
    {
        LM out = aref[0].m_lm;
        for (int i = 1; i < carg; ++i)
        {
            if (!IsNumber(aref[i]))
                continue;

            const float inverse = 1.0f / RefAsF32(aref[i]);
            out.gMin *= inverse;
            out.gMax *= inverse;
        }

        ret.SetLm(out);
        return ret;
    }

    bool forceFloat = IsFloat(aref[0]);
    float total = RefAsF32(aref[0]);

    for (int i = 1; i < carg; ++i)
    {
        if (!IsNumber(aref[i]))
            continue;

        forceFloat = forceFloat || IsFloat(aref[i]);
        total /= RefAsF32(aref[i]);
    }

    return MakeNumber(total, forceFloat);
}

CRef RefOpPrint(int carg, CRef* aref, CFrame*)
{
    // Original decompile only returns Void. Add printing here if you wire RefOpPrint.
    (void)carg;
    (void)aref;
    return MakeVoidRef();
}

CRef RefOpPrintFrame(int, CRef*, CFrame*)
{
    return MakeVoidRef();
}

CRef RefOpPrintSidebag(int, CRef*, CFrame*)
{
    return MakeVoidRef();
}

CRef RefOpIntEqual(int carg, CRef* aref, CFrame*)
{
    if (!aref || carg < 2)
        return MakeBoolRef(false);

    const int first = RefAsS32(aref[0]);
    for (int i = 1; i < carg; ++i)
    {
        if (RefAsS32(aref[i]) != first)
            return MakeBoolRef(false);
    }

    return MakeBoolRef(true);
}

CRef RefOpL(int carg, CRef* aref, CFrame*)
{
    return CompareNumbers(carg, aref, [](float a, float b) { return a < b; });
}

CRef RefOpLE(int carg, CRef* aref, CFrame*)
{
    return CompareNumbers(carg, aref, [](float a, float b) { return a <= b; });
}

CRef RefOpG(int carg, CRef* aref, CFrame*)
{
    return CompareNumbers(carg, aref, [](float a, float b) { return a > b; });
}

CRef RefOpGE(int carg, CRef* aref, CFrame*)
{
    return CompareNumbers(carg, aref, [](float a, float b) { return a >= b; });
}

CRef RefOpEqv(int carg, CRef* aref, CFrame*)
{
    if (!aref || carg < 2)
        return MakeBoolRef(false);

    const CRef& a = aref[0];
    const CRef& b = aref[1];

    if (a.m_tagk != b.m_tagk)
        return MakeBoolRef(false);

    switch (a.m_tagk)
    {
    case TAGK_None:
    case TAGK_Set:
    case TAGK_Define:
    case TAGK_DefineMacro:
    case TAGK_DefineMacroLambda:
    case TAGK_Assert:
    case TAGK_If:
    case TAGK_Or:
    case TAGK_And:
    case TAGK_Cond:
    case TAGK_Else:
    case TAGK_Case:
    case TAGK_Let:
    case TAGK_While:
    case TAGK_Lambda:
    case TAGK_Begin:
    case TAGK_Import:
    case TAGK_Quote:
    case TAGK_Dot:
    case TAGK_Pipe:
    case TAGK_Void:
        return MakeBoolRef(true);

    case TAGK_S32:
        return MakeBoolRef(a.m_n == b.m_n);

    case TAGK_Bool:
        return MakeBoolRef(a.m_bool == b.m_bool);

    case TAGK_Symid:
        return MakeBoolRef(a.m_symid == b.m_symid);

    case TAGK_Bifk:
        return MakeBoolRef(a.m_bifk == b.m_bifk);

    case TAGK_Pair:
        return MakeBoolRef(a.m_ppair == b.m_ppair);

    case TAGK_Proc:
        return MakeBoolRef(a.m_pproc == b.m_pproc);

    case TAGK_Basic:
        return MakeBoolRef(a.m_pbasic == b.m_pbasic);

    case TAGK_Method:
        return MakeBoolRef(a.m_pmethod == b.m_pmethod);

    default:
        return MakeBoolRef(false);
    }
}

CRef RefOpEqual(int carg, CRef* aref, CFrame*)
{
    if (!aref || carg < 2)
        return MakeBoolRef(false);

    return MakeBoolRef(RefDeepEqual(aref[0], aref[1]));
}

CRef RefOpIsBoolean(int carg, CRef* aref, CFrame*)
{
    return MakeBoolRef(aref && carg > 0 && aref[0].m_tagk == TAGK_Bool);
}

CRef RefOpIsNum(int carg, CRef* aref, CFrame*)
{
    return MakeBoolRef(aref && carg > 0 && IsNumber(aref[0]));
}

CRef RefOpIsInteger(int carg, CRef* aref, CFrame*)
{
    return MakeBoolRef(aref && carg > 0 && aref[0].m_tagk == TAGK_S32);
}

CRef RefOpIsFloat(int carg, CRef* aref, CFrame*)
{
    return MakeBoolRef(aref && carg > 0 && aref[0].m_tagk == TAGK_F32);
}

CRef RefOpIsSymbol(int carg, CRef* aref, CFrame*)
{
    return MakeBoolRef(aref && carg > 0 && aref[0].m_tagk == TAGK_Symid);
}

CRef RefOpIsVector(int carg, CRef* aref, CFrame*)
{
    return MakeBoolRef(aref && carg > 0 && aref[0].m_tagk == TAGK_Vector);
}

CRef RefOpIsMatrix(int carg, CRef* aref, CFrame*)
{
    return MakeBoolRef(carg >= 1 && aref[0].m_tagk == TAGK_Matrix);
}

// -----------------------------------------------------
// 22. RefOpIsClq
// -----------------------------------------------------

CRef RefOpIsClq(int carg, CRef* aref, CFrame*)
{
    return MakeBoolRef(carg >= 1 && aref[0].m_tagk == TAGK_Clq);
}

// -----------------------------------------------------
// 23. RefOpIsLm
// -----------------------------------------------------

CRef RefOpIsLm(int carg, CRef* aref, CFrame*)
{
    return MakeBoolRef(carg >= 1 && aref[0].m_tagk == TAGK_Lm);
}

// -----------------------------------------------------
// 24. RefOpIsSmp
// -----------------------------------------------------

CRef RefOpIsSmp(int carg, CRef* aref, CFrame*)
{
    return MakeBoolRef(carg >= 1 && aref[0].m_tagk == TAGK_Smp);
}

// -----------------------------------------------------
// 25. RefOpIsList
// -----------------------------------------------------

CRef RefOpIsList(int carg, CRef* aref, CFrame*)
{
    if (carg < 1)
        return MakeBoolRef(false);

    return MakeBoolRef(aref[0].m_tagk == TAGK_Pair ||
        aref[0].m_tagk == TAGK_None ||
        aref[0].m_tagk == TAGK_Nil);
}

// -----------------------------------------------------
// 26. RefOpIsNull
// -----------------------------------------------------

CRef RefOpIsNull(int carg, CRef* aref, CFrame*)
{
    if (carg < 1)
        return MakeBoolRef(true);

    return MakeBoolRef(aref[0].m_tagk == TAGK_Nil ||
        aref[0].m_tagk == TAGK_None ||
        (aref[0].m_tagk == TAGK_Pair && !aref[0].m_ppair));
}

// -----------------------------------------------------
// 27. RefOpIsObject
// -----------------------------------------------------

CRef RefOpIsObject(int carg, CRef* aref, CFrame*)
{
    // In the original tag table this corresponds to TAGK_Basic/object.
    return MakeBoolRef(carg >= 1 && aref[0].m_tagk == TAGK_Basic);
}

// -----------------------------------------------------
// 28. RefOpIsNullObj
// -----------------------------------------------------

CRef RefOpIsNullObj(int carg, CRef* aref, CFrame*)
{
    if (carg < 1 || aref[0].m_tagk != TAGK_Basic)
        return MakeBoolRef(false);

    return MakeBoolRef(aref[0].m_pbasic == nullptr);
}

// -----------------------------------------------------
// 29. RefOpIsMethod
// -----------------------------------------------------

CRef RefOpIsMethod(int carg, CRef* aref, CFrame*)
{
    return MakeBoolRef(carg >= 1 && aref[0].m_tagk == TAGK_Method);
}

// -----------------------------------------------------
// 30. RefOpIsProcedure
// -----------------------------------------------------

CRef RefOpIsProcedure(int carg, CRef* aref, CFrame*)
{
    if (carg < 1)
        return MakeBoolRef(false);

    return MakeBoolRef(aref[0].m_tagk == TAGK_Proc ||
        aref[0].m_tagk == TAGK_Bifk ||
        aref[0].m_tagk == TAGK_Method);
}

// -----------------------------------------------------
// 31. RefOpAreNear
// -----------------------------------------------------

CRef RefOpAreNear(int carg, CRef* aref, CFrame*)
{
    if (carg < 3)
        return MakeBoolRef(false);

    const float a = RefAsF32(aref[0]);
    const float b = RefAsF32(aref[1]);
    const float epsilon = RefAsF32(aref[2]);

    return MakeBoolRef(FloatsNear(a, b, epsilon));
}

// -----------------------------------------------------
// 32. RefOpNot
// -----------------------------------------------------

CRef RefOpNot(int carg, CRef* aref, CFrame*)
{
    if (carg < 1)
        return MakeBoolRef(true);

    // Original behavior only treats Bool false as false.
    return MakeBoolRef(IsFalse(aref[0]));
}

// -----------------------------------------------------
// 33. RefOpCons
// -----------------------------------------------------

CRef RefOpCons(int carg, CRef* aref, CFrame*)
{
    if (carg < 1)
        return MakeNoneRef();

    auto node = MakePairNode(aref[0]);

    if (carg >= 2)
    {
        if (aref[1].m_tagk == TAGK_Pair)
            node->m_ppairNext = aref[1].m_ppair;
        else
            node->m_ppairNext = nullptr;
    }

    CRef ret;
    ret.SetPair(node);
    return ret;
}

// -----------------------------------------------------
// 34. RefOpCar
// -----------------------------------------------------

CRef RefOpCar(int carg, CRef* aref, CFrame*)
{
    if (carg < 1 || aref[0].m_tagk != TAGK_Pair || !aref[0].m_ppair)
        return MakeNoneRef();

    return aref[0].m_ppair->m_ref;
}

// -----------------------------------------------------
// 35. RefOpCdr
// -----------------------------------------------------

CRef RefOpCdr(int carg, CRef* aref, CFrame*)
{
    if (carg < 1 || aref[0].m_tagk != TAGK_Pair || !aref[0].m_ppair)
        return MakeNoneRef();

    CRef ret;
    if (aref[0].m_ppair->m_ppairNext)
        ret.SetPair(aref[0].m_ppair->m_ppairNext);
    else
        ret.SetTag(TAGK_None);

    return ret;
}

// -----------------------------------------------------
// 36. RefOpSetCar
// -----------------------------------------------------

CRef RefOpSetCar(int carg, CRef* aref, CFrame*)
{
    if (carg >= 2 && aref[0].m_tagk == TAGK_Pair && aref[0].m_ppair)
        aref[0].m_ppair->m_ref = aref[1];

    return MakeVoidRef();
}

// -----------------------------------------------------
// 37. RefOpSetCdr
// -----------------------------------------------------

CRef RefOpSetCdr(int carg, CRef* aref, CFrame*)
{
    if (carg >= 2 && aref[0].m_tagk == TAGK_Pair && aref[0].m_ppair)
    {
        if (aref[1].m_tagk == TAGK_Pair)
            aref[0].m_ppair->m_ppairNext = aref[1].m_ppair;
        else
            aref[0].m_ppair->m_ppairNext = nullptr;
    }

    return MakeVoidRef();
}

// -----------------------------------------------------
// 38. RefOpLength
// -----------------------------------------------------

CRef RefOpLength(int carg, CRef* aref, CFrame*)
{
    int length = 0;

    if (carg >= 1 && aref[0].m_tagk == TAGK_Pair)
    {
        for (auto node = aref[0].m_ppair; node; node = node->m_ppairNext)
            ++length;
    }

    CRef ret;
    ret.SetS32(length);
    return ret;
}

// -----------------------------------------------------
// 39. RefOpNth
// -----------------------------------------------------

CRef RefOpNth(int carg, CRef* aref, CFrame*)
{
    if (carg < 2 || aref[0].m_tagk != TAGK_Pair)
        return MakeNoneRef();

    const int wanted = RefAsS32(aref[1]);
    if (wanted < 0)
        return MakeNoneRef();

    int index = 0;
    for (auto node = aref[0].m_ppair; node; node = node->m_ppairNext, ++index)
    {
        if (index == wanted)
            return node->m_ref;
    }

    return MakeNoneRef();
}

// -----------------------------------------------------
// 40. RefOpIsMember
// -----------------------------------------------------

CRef RefOpIsMember(int carg, CRef* aref, CFrame*)
{
    if (carg < 2 || aref[1].m_tagk != TAGK_Pair)
        return MakeBoolRef(false);

    for (auto node = aref[1].m_ppair; node; node = node->m_ppairNext)
    {
        if (aref[0] == node->m_ref)
            return MakeBoolRef(true);
    }

    return MakeBoolRef(false);
}

// -----------------------------------------------------
// 41. RefOpList
// -----------------------------------------------------

CRef RefOpList(int carg, CRef* aref, CFrame*)
{
    std::shared_ptr<CPair> head;
    std::shared_ptr<CPair> tail;

    for (int i = 0; i < carg; ++i)
    {
        auto node = MakePairNode(aref[i]);

        if (!head)
            head = node;
        else
            tail->m_ppairNext = node;

        tail = node;
    }

    CRef ret;
    if (head)
        ret.SetPair(head);
    else
        ret.SetTag(TAGK_None);

    return ret;
}

CRef RefOpAppend(int carg, CRef* aref, CFrame*)
{
    if (carg <= 0)
        return MakeNoneRef();

    std::shared_ptr<CPair> outHead;
    std::shared_ptr<CPair> outTail;

    for (int i = 0; i < carg; ++i)
    {
        const CRef& arg = aref[i];

        if (arg.m_tagk == TAGK_None || arg.m_tagk == TAGK_Nil)
            continue;

        if (arg.m_tagk != TAGK_Pair || !arg.m_ppair)
            continue;

        auto copied = CopyList(arg.m_ppair);
        if (!copied)
            continue;

        if (!outHead)
        {
            outHead = copied;
        }
        else
        {
            outTail->m_ppairNext = copied;
        }

        outTail = copied;
        while (outTail->m_ppairNext)
            outTail = outTail->m_ppairNext;
    }

    return MakePairRef(outHead);
}

// -----------------------------------------------------
// 43. RefOpMap
// -----------------------------------------------------

CRef RefOpMap(int carg, CRef* aref, CFrame* pframe)
{
    if (carg < 2 || !IsListLike(aref[1]))
        return MakeNoneRef();

    const CRef& callable = aref[0];
    auto input = aref[1].m_ppair;

    std::shared_ptr<CPair> outHead;
    std::shared_ptr<CPair> outTail;

    for (auto node = input; node; node = node->m_ppairNext)
    {
        CRef mapped = EvalCall2(callable, node->m_ref, pframe);
        auto outNode = MakePairNode(mapped);
        AppendNode(outHead, outTail, outNode);
    }

    return MakePairRef(outHead);
}

// -----------------------------------------------------
// 44. RefOpFilter
// -----------------------------------------------------

CRef RefOpFilter(int carg, CRef* aref, CFrame* pframe)
{
    if (carg < 2 || !IsListLike(aref[1]))
        return MakeNoneRef();

    const CRef& callable = aref[0];
    auto input = aref[1].m_ppair;

    std::shared_ptr<CPair> outHead;
    std::shared_ptr<CPair> outTail;

    for (auto node = input; node; node = node->m_ppairNext)
    {
        CRef keep = EvalCall2(callable, node->m_ref, pframe);

        if (!IsFalse(keep))
        {
            auto outNode = MakePairNode(node->m_ref);
            AppendNode(outHead, outTail, outNode);
        }
    }

    return MakePairRef(outHead);
}

// -----------------------------------------------------
// 45. RefOpForEach
// -----------------------------------------------------

CRef RefOpForEach(int carg, CRef* aref, CFrame* pframe)
{
    if (carg < 2 || !IsListLike(aref[1]))
        return MakeVoidRef();

    const CRef& callable = aref[0];
    auto input = aref[1].m_ppair;

    for (auto node = input; node; node = node->m_ppairNext)
        EvalCall2(callable, node->m_ref, pframe);

    return MakeVoidRef();
}

// -----------------------------------------------------
// 46. RefOpEval
// -----------------------------------------------------

CRef RefOpEval(int carg, CRef* aref, CFrame* pframe)
{
    if (carg < 1)
        return MakeVoidRef();

    auto expr = PpairNew();
    expr->m_ref = aref[0];
    return RefEval(expr.get(), pframe);
}

// -----------------------------------------------------
// 47. RefOpVector
// -----------------------------------------------------

CRef RefOpVector(int carg, CRef* aref, CFrame*)
{
    glm::vec3 v{};

    if (carg > 0) v.x = RefAsF32(aref[0]);
    if (carg > 1) v.y = RefAsF32(aref[1]);
    if (carg > 2) v.z = RefAsF32(aref[2]);

    CRef ret;
    ret.SetVector(v);
    return ret;
}

// -----------------------------------------------------
// 48. RefOpMatrix
// -----------------------------------------------------

CRef RefOpMatrix(int carg, CRef* aref, CFrame*)
{
    glm::mat4 m{ 1.0f };

    // The original accepts exactly 9 scalars for a 3x3 rotation matrix or
    // exactly 16 scalars for a full 4x4 matrix.  Its 3x3 path starts from an
    // identity 4x4 so the homogeneous row/column remain valid.
    if (carg == 9)
    {
        int argument = 0;
        for (int row = 0; row < 3; ++row)
            for (int column = 0; column < 3; ++column)
                m[row][column] = RefAsF32(aref[argument++]);
    }
    else if (carg == 16)
    {
        float* p = &m[0][0];
        for (int i = 0; i < 16; ++i)
            p[i] = RefAsF32(aref[i]);
    }
    else if (carg == 3 || carg == 4)
    {
        for (int i = 0; i < carg; ++i)
        {
            if (aref[i].m_tagk == TAGK_Vector)
            {
                m[i][0] = aref[i].m_vector4.x;
                m[i][1] = aref[i].m_vector4.y;
                m[i][2] = aref[i].m_vector4.z;
                m[i][3] = aref[i].m_vector4.w;
            }
        }
    }

    CRef ret;
    ret.SetMatrix(m);
    return ret;
}

// -----------------------------------------------------
// 49. RefOpClq
// -----------------------------------------------------

CRef RefOpClq(int carg, CRef* aref, CFrame*)
{
    CLQ clq{};

    if (carg > 0) clq.g0 = RefAsF32(aref[0]);
    if (carg > 1) clq.g1 = RefAsF32(aref[1]);
    if (carg > 2) clq.g2 = RefAsF32(aref[2]);

    CRef ret;
    ret.SetClq(clq);
    return ret;
}

// -----------------------------------------------------
// 50. RefOpLm
// -----------------------------------------------------

CRef RefOpLm(int carg, CRef* aref, CFrame*)
{
    LM lm{};

    if (carg > 0) lm.gMin = RefAsF32(aref[0]);
    if (carg > 1) lm.gMax = RefAsF32(aref[1]);

    CRef ret;
    ret.SetLm(lm);
    return ret;
}

// -----------------------------------------------------
// 51. RefOpSmp
// -----------------------------------------------------

CRef RefOpSmp(int carg, CRef* aref, CFrame*)
{
    SMP smp{};

    if (carg > 0) smp.svSlow = RefAsF32(aref[0]);
    if (carg > 1) smp.svFast = RefAsF32(aref[1]);
    if (carg > 2) smp.dtFast = RefAsF32(aref[2]);

    CRef ret;
    ret.SetSmp(smp);
    return ret;
}

// -----------------------------------------------------
// 52. RefOpGetElement
// -----------------------------------------------------

CRef RefOpGetElement(int carg, CRef* aref, CFrame*)
{
    if (carg < 2)
        return MakeNoneRef();

    const CRef& container = aref[0];
    const int index = RefAsS32(aref[1]);

    CRef ret;
    ret.SetTag(TAGK_None);

    switch (container.m_tagk)
    {
    case TAGK_Vector:
        if (index == 0) ret.SetF32(container.m_vector.x);
        else if (index == 1) ret.SetF32(container.m_vector.y);
        else if (index == 2) ret.SetF32(container.m_vector.z);
        break;

    case TAGK_Matrix:
        if (carg >= 3)
        {
            int col = RefAsS32(aref[2]);
            if (index >= 0 && index < 4 && col >= 0 && col < 4)
                ret.SetF32(container.m_matrix[index][col]);
        }
        else if (index >= 0 && index < 4)
        {
            glm::vec3 row{};
            row.x = container.m_matrix[index][0];
            row.y = container.m_matrix[index][1];
            row.z = container.m_matrix[index][2];
            ret.SetVector(row);
        }
        break;

    case TAGK_Clq:
        if (index == 0) ret.SetF32(container.m_clq.g0);
        else if (index == 1) ret.SetF32(container.m_clq.g1);
        else if (index == 2) ret.SetF32(container.m_clq.g2);
        break;

    case TAGK_Lm:
        if (index == 0) ret.SetF32(container.m_lm.gMin);
        else if (index == 1) ret.SetF32(container.m_lm.gMax);
        break;

    case TAGK_Smp:
        if (index == 0) ret.SetF32(container.m_smp.svSlow);
        else if (index == 1) ret.SetF32(container.m_smp.svFast);
        else if (index == 2) ret.SetF32(container.m_smp.dtFast);
        break;

    default:
        break;
    }

    return ret;
}

// -----------------------------------------------------
// 53. RefOpRandomSeed
// -----------------------------------------------------

CRef RefOpRandomSeed(int carg, CRef* aref, CFrame*)
{
    unsigned int seed = 0;

    if (carg > 0)
        seed = static_cast<unsigned int>(RefAsS32(aref[0]));

    std::srand(seed);
    return MakeVoidRef();
}

// -----------------------------------------------------
// 54. RefOpRandom
// -----------------------------------------------------

CRef RefOpRandom(int carg, CRef* aref, CFrame*)
{
    float low = 0.0f;
    float high = 1.0f;

    if (carg == 1)
    {
        high = RefAsF32(aref[0]);
    }
    else if (carg >= 2)
    {
        low = RefAsF32(aref[0]);
        high = RefAsF32(aref[1]);
    }

    CRef ret;
    ret.SetF32(low + (high - low) * RandomFloat01());
    return ret;
}

// -----------------------------------------------------
// 55. RefOpSqrt
// -----------------------------------------------------

CRef RefOpSqrt(int carg, CRef* aref, CFrame*)
{
    CRef ret;
    ret.SetF32(carg > 0 ? std::sqrt(RefAsF32(aref[0])) : 0.0f);
    return ret;
}

// -----------------------------------------------------
// 56. RefOpSin
// -----------------------------------------------------

CRef RefOpSin(int carg, CRef* aref, CFrame*)
{
    CRef ret;
    ret.SetF32(carg > 0 ? std::sin(RefAsF32(aref[0])) : 0.0f);
    return ret;
}

// -----------------------------------------------------
// 57. RefOpCos
// -----------------------------------------------------

CRef RefOpCos(int carg, CRef* aref, CFrame*)
{
    CRef ret;
    ret.SetF32(carg > 0 ? std::cos(RefAsF32(aref[0])) : 0.0f);
    return ret;
}

// -----------------------------------------------------
// 58. RefOpTan
// -----------------------------------------------------

CRef RefOpTan(int carg, CRef* aref, CFrame*)
{
    CRef ret;
    ret.SetF32(carg > 0 ? std::tan(RefAsF32(aref[0])) : 0.0f);
    return ret;
}

// -----------------------------------------------------
// 59. RefOpAsin
// -----------------------------------------------------

CRef RefOpAsin(int carg, CRef* aref, CFrame*)
{
    CRef ret;
    ret.SetF32(carg > 0 ? std::asin(RefAsF32(aref[0])) : 0.0f);
    return ret;
}

// -----------------------------------------------------
// 60. RefOpAcos
// -----------------------------------------------------

CRef RefOpAcos(int carg, CRef* aref, CFrame*)
{
    CRef ret;
    ret.SetF32(carg > 0 ? std::acos(RefAsF32(aref[0])) : 0.0f);
    return ret;
}

CRef RefOpAtan(int argc, CRef* argv, CFrame* pframe)
{
    // Original dispatches through RefUfo.
    // atan(x)
    CRef ret;

    if (!HasArg(argc, 0))
    {
        ret.SetF32(0.0f);
        return ret;
    }

    ret.SetF32(std::atan(RefAsF32(argv[0])));
    return ret;
}

// =====================================================
// 62. RefOpRadNormalize
// =====================================================

CRef RefOpRadNormalize(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (!HasArg(argc, 0))
    {
        ret.SetF32(0.0f);
        return ret;
    }

    float radians = RefAsF32(argv[0]);

    constexpr float twoPi = 6.28318530717958647692f;

    while (radians > 3.14159265358979323846f)
        radians -= twoPi;

    while (radians < -3.14159265358979323846f)
        radians += twoPi;

    ret.SetF32(radians);
    return ret;
}

// =====================================================
// 63. RefOpAtan2
// =====================================================

CRef RefOpAtan2(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    const float y = HasArg(argc, 0) ? RefAsF32(argv[0]) : 0.0f;
    const float x = HasArg(argc, 1) ? RefAsF32(argv[1]) : 0.0f;

    ret.SetF32(std::atan2(y, x));
    return ret;
}

// =====================================================
// 64. RefOpVectorDotProduct
// =====================================================

CRef RefOpVectorDotProduct(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 2)
    {
        ret.SetF32(0.0f);
        return ret;
    }

    const glm::vec3 a = RefAsVector(argv[0]);
    const glm::vec3 b = RefAsVector(argv[1]);

    ret.SetF32(glm::dot(a, b));
    return ret;
}

// =====================================================
// 65. RefOpVectorCrossProduct
// =====================================================

CRef RefOpVectorCrossProduct(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 2)
    {
        ret.SetVector(glm::vec3{});
        return ret;
    }

    const glm::vec3 a = RefAsVector(argv[0]);
    const glm::vec3 b = RefAsVector(argv[1]);

    ret.SetVector(glm::cross(a, b));
    return ret;
}

// =====================================================
// 66. RefOpVectorLth
// =====================================================

CRef RefOpVectorLth(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 1)
    {
        ret.SetF32(0.0f);
        return ret;
    }

    ret.SetF32(glm::length(RefAsVector(argv[0])));
    return ret;
}

// =====================================================
// 67. RefOpVectorDistance
// =====================================================

CRef RefOpVectorDistance(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 1)
    {
        ret.SetF32(0.0f);
        return ret;
    }

    const glm::vec3 a = RefAsVector(argv[0]);
    const glm::vec3 b = HasArg(argc, 1) ? RefAsVector(argv[1]) : glm::vec3{};

    ret.SetF32(glm::distance(a, b));
    return ret;
}

// =====================================================
// 68. RefOpVectorDistanceSquared
// =====================================================

CRef RefOpVectorDistanceSquared(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 1)
    {
        ret.SetF32(0.0f);
        return ret;
    }

    const glm::vec3 a = RefAsVector(argv[0]);
    const glm::vec3 b = HasArg(argc, 1) ? RefAsVector(argv[1]) : glm::vec3{};
    const glm::vec3 d = a - b;

    ret.SetF32(glm::dot(d, d));
    return ret;
}

// =====================================================
// 69. RefOpVectorNormalize
// =====================================================

CRef RefOpVectorNormalize(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 1)
    {
        ret.SetVector(glm::vec3{});
        return ret;
    }

    const glm::vec3 v = RefAsVector(argv[0]);
    const float len = glm::length(v);

    if (len >= 0.0001f)
    {
        ret.SetVector(v / len);
    }
    else
    {
        // Original used zero vector unless a fallback vector was passed.
        const glm::vec3 fallback = HasArg(argc, 1) ? RefAsVector(argv[1]) : glm::vec3{};
        ret.SetVector(fallback);
    }

    return ret;
}

// =====================================================
// 70. RefOpVectorProjectNormal
// =====================================================

CRef RefOpVectorProjectNormal(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 2)
    {
        ret.SetVector(glm::vec3{});
        return ret;
    }

    const glm::vec3 v = RefAsVector(argv[0]);
    const glm::vec3 normal = RefAsVector(argv[1]);

    // Original assumes the normal is already normalized.
    ret.SetVector(normal * glm::dot(v, normal));
    return ret;
}

// =====================================================
// 71. RefOpVectorProjectTangent
// =====================================================

CRef RefOpVectorProjectTangent(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 2)
    {
        ret.SetVector(glm::vec3{});
        return ret;
    }

    const glm::vec3 v = RefAsVector(argv[0]);
    const glm::vec3 normal = RefAsVector(argv[1]);

    // Tangent projection = original vector - normal projection.
    ret.SetVector(v - normal * glm::dot(v, normal));
    return ret;
}

// =====================================================
// 72. RefOpVectorBallisticVelocity
// =====================================================

CRef RefOpVectorBallisticVelocity(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 3)
    {
        ret.SetVector(glm::vec3{});
        return ret;
    }

    const glm::vec3 launch = RefAsVector(argv[0]);
    const glm::vec3 target = RefAsVector(argv[1]);
    const float speed = RefAsF32(argv[2]);

    glm::vec3 muzzle{};

    // Original optionally passes an SO* launch object.
    // This cleaned version passes nullptr unless you wire the object source back in.
    SO* launchObject = nullptr;

    if (!FCalculateMuzzleVelocity(launch, target, speed, muzzle, launchObject))
        muzzle = glm::vec3{};

    ret.SetVector(muzzle);
    return ret;
}

// =====================================================
// 73. RefOpVectorRadianNormal
// =====================================================

CRef RefOpVectorRadianNormal(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 2)
    {
        ret.SetTag(TAGK_None);
        return ret;
    }

    const glm::vec3 a = RefAsVector(argv[0]);
    const glm::vec3 b = RefAsVector(argv[1]);

    glm::vec3 normal{};
    const float radians = RadBetweenVectors(a, b, &normal);

    auto head = PpairNew();
    auto tail = PpairNew();

    head->m_ref.SetF32(radians);
    tail->m_ref.SetVector(normal);

    head->m_ppairNext = tail;

    ret.SetPair(head);
    return ret;
}

// =====================================================
// 74. RefOpMatrixTranspose
// =====================================================

CRef RefOpMatrixTranspose(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 1)
    {
        ret.SetMatrix(glm::mat4{ 1.0f });
        return ret;
    }

    ret.SetMatrix(glm::transpose(RefAsMatrix(argv[0])));
    return ret;
}

// =====================================================
// 75. RefOpMatrixInvert
// =====================================================

CRef RefOpMatrixInvert(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 1)
    {
        ret.SetMatrix(glm::mat4{ 1.0f });
        return ret;
    }

    ret.SetMatrix(glm::inverse(RefAsMatrix(argv[0])));
    return ret;
}

// =====================================================
// 76. RefOpMatrixCalculateDmat
// =====================================================

CRef RefOpMatrixCalculateDmat(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 2)
    {
        ret.SetMatrix(glm::mat4{ 1.0f });
        return ret;
    }

    glm::mat4 dmat{ 1.0f };
    CalculateDmat4(RefAsMatrix(argv[0]), RefAsMatrix(argv[1]), dmat);

    ret.SetMatrix(dmat);
    return ret;
}

// =====================================================
// 77. RefOpMatrixInterpolateRotate
// =====================================================

CRef RefOpMatrixInterpolateRotate(int carg, CRef* aref, CFrame* pframe)
{
    CRef ret;

    if (carg < 3)
    {
        ret.SetMatrix(glm::mat4{ 1.0f });
        return ret;
    }

    glm::mat4 from4 = RefAsMatrix(aref[0]);
    glm::mat4 to4 = RefAsMatrix(aref[1]);
    float t = RefAsF32(aref[2]);

    glm::mat3 from3(from4);
    glm::mat3 to3(to4);

    glm::mat3 dmat{ 1.0f };
    CalculateDmat(&from3, &to3, &dmat);

    float radians = 0.0f;
    glm::vec3 normal{ 0.0f, 0.0f, 1.0f };
    DecomposeRotateMatrixRad(&dmat, &radians, &normal);

    glm::mat3 rotation{ 1.0f };
    LoadRotateMatrixRad(radians * t, &normal, &rotation);

    glm::mat3 result3 = from3 * rotation;

    glm::mat4 result4 = from4;
    result4[0] = glm::vec4(result3[0], 0.0f);
    result4[1] = glm::vec4(result3[1], 0.0f);
    result4[2] = glm::vec4(result3[2], 0.0f);

    ret.SetMatrix(result4);
    return ret;
}

// =====================================================
// 78. RefOpMatrixDecomposeToTranslate
// =====================================================

CRef RefOpMatrixDecomposeToTranslate(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 1)
    {
        ret.SetVector(glm::vec3{});
        return ret;
    }

    const glm::mat4 m = RefAsMatrix(argv[0]);

    // GLM is column-major: translation lives in column 3.
    ret.SetVector(glm::vec3(m[3]));
    return ret;
}

// =====================================================
// 79. RefOpMatrixDecomposeToRotate
// =====================================================

CRef RefOpMatrixDecomposeToRotate(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 1)
    {
        ret.SetMatrix(glm::mat4{ 1.0f });
        return ret;
    }

    glm::mat4 m = RefAsMatrix(argv[0]);

    // Strip translation.
    m[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    ret.SetMatrix(m);
    return ret;
}

// =====================================================
// 80. RefOpMatrixDecomposeToEuler
// =====================================================

CRef RefOpMatrixDecomposeToEuler(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 1)
    {
        ret.SetVector(glm::vec3{});
        return ret;
    }

    glm::vec3 euler{};
    DecomposeRotateMatrixEuler(RefAsMatrix(argv[0]), euler);

    ret.SetVector(euler);
    return ret;
}

// =====================================================
// 81. RefOpMatrixDecomposeToRadianNormal
// =====================================================

CRef RefOpMatrixDecomposeToRadianNormal(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 1)
    {
        ret.SetTag(TAGK_None);
        return ret;
    }

    float radians = 0.0f;
    glm::vec3 normal{};

    glm::mat3 mat = RefAsMatrix(argv[0]);
    DecomposeRotateMatrixRad(&mat, &radians, &normal);

    auto head = PpairNew();
    auto tail = PpairNew();

    head->m_ref.SetF32(radians);
    tail->m_ref.SetVector(normal);

    head->m_ppairNext = tail;

    ret.SetPair(head);
    return ret;
}

// =====================================================
// 82. RefOpMatrixLookAt
// =====================================================

CRef RefOpMatrixLookAt(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 2)
    {
        ret.SetMatrix(glm::mat4{ 1.0f });
        return ret;
    }

    glm::vec3 from = RefAsVector(argv[0]);
    glm::vec3 target = RefAsVector(argv[1]);
    glm::vec3 up = HasArg(argc, 2)
        ? RefAsVector(argv[2])
        : glm::vec3{ 0.0f, 0.0f, 1.0f };

    glm::mat3 rotation{ 1.0f };
    glm::vec3 result = target - from;
    BuildOrthonormalMatrixZ(result, up, rotation);

    glm::mat4 matrix{ 1.0f };
    LoadMatrixFromPosRot(&from, &rotation, &matrix);

    ret.SetMatrix(matrix);
    return ret;
}

// =====================================================
// 83. RefOpMatrixTiltUpright
// =====================================================

CRef RefOpMatrixTiltUpright(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 1)
    {
        ret.SetMatrix(glm::mat4{ 1.0f });
        return ret;
    }

    glm::mat4 src4 = RefAsMatrix(argv[0]);

    glm::mat3 srcRot(src4);
    glm::vec3 pos(src4[3]);

    glm::mat3 uprightRot{ 1.0f };
    TiltMatUpright(&srcRot, nullptr, &uprightRot);

    glm::mat4 out{ 1.0f };
    LoadMatrixFromPosRot(&pos, &uprightRot, &out);

    ret.SetMatrix(out);
    return ret;
}

// =====================================================
// 84. RefOpClqEvaluate
// =====================================================

CRef RefOpClqEvaluate(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 2)
    {
        ret.SetF32(0.0f);
        return ret;
    }

    const CLQ clq = RefAsClq(argv[0]);
    const float x = RefAsF32(argv[1]);

    // Original formula:
    // g0 + x * (g1 + x * g2)
    ret.SetF32(clq.g0 + x * (clq.g1 + x * clq.g2));
    return ret;
}

// =====================================================
// 85. RefOpClqEvaluateLm
// =====================================================

CRef RefOpClqEvaluateLm(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 3)
    {
        ret.SetF32(0.0f);
        return ret;
    }

    const CLQ clq = RefAsClq(argv[0]);
    const LM lm = RefAsLm(argv[1]);
    const float x = RefAsF32(argv[2]);

    const float unclamped = clq.g0 + x * (clq.g1 + x * clq.g2);
    ret.SetF32(glm::clamp(unclamped, lm.gMin, lm.gMax));
    return ret;
}

// =====================================================
// 86. RefOpClqFit
// =====================================================

CRef RefOpClqFit(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 4)
    {
        ret.SetClq(CLQ{});
        return ret;
    }

    CLQ clq{};
    FitClq(RefAsF32(argv[0]), RefAsF32(argv[1]), RefAsF32(argv[2]), RefAsF32(argv[3]), &clq);

    ret.SetClq(clq);
    return ret;
}

// =====================================================
// 87. RefOpLmLimit
// =====================================================

CRef RefOpLmLimit(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 2)
    {
        ret.SetF32(0.0f);
        return ret;
    }

    LM lm = RefAsLm(argv[0]);
    float value = RefAsF32(argv[1]);

    ret.SetF32(GLimitLm(&lm, value));
    return ret;
}

// =====================================================
// 88. RefOpLmCheck
// =====================================================

CRef RefOpLmCheck(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 2)
    {
        ret.SetBool(0);
        return ret;
    }

    LM lm = RefAsLm(argv[0]);
    ret.SetBool(FCheckLm(&lm, RefAsF32(argv[1])) ? 1 : 0);
    return ret;
}

// =====================================================
// 89. RefOpFloor
// =====================================================

CRef RefOpFloor(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;
    ret.SetS32(argc > 0 ? static_cast<s32>(std::floor(RefAsF32(argv[0]))) : 0);
    return ret;
}

// =====================================================
// 90. RefOpCeiling
// =====================================================

CRef RefOpCeiling(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;
    ret.SetS32(argc > 0 ? static_cast<s32>(std::ceil(RefAsF32(argv[0]))) : 0);
    return ret;
}

// =====================================================
// 91. RefOpRound
// =====================================================

CRef RefOpRound(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;
    ret.SetS32(argc > 0 ? static_cast<s32>(std::floor(RefAsF32(argv[0]) + 0.5f)) : 0);
    return ret;
}

// =====================================================
// 92. RefOpTruncate
// =====================================================

CRef RefOpTruncate(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;
    ret.SetS32(argc > 0 ? static_cast<s32>(RefAsF32(argv[0])) : 0);
    return ret;
}

// =====================================================
// 93. RefOpAbs
// =====================================================

CRef RefOpAbs(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc <= 0)
    {
        ret.SetS32(0);
        return ret;
    }

    if (argv[0].m_tagk == TAGK_S32)
        ret.SetS32(static_cast<s32>(std::abs(argv[0].m_n)));
    else
        ret.SetF32(std::fabs(RefAsF32(argv[0])));

    return ret;
}

// =====================================================
// 94. RefOpMaximum
// =====================================================

CRef RefOpMaximum(int argc, CRef* argv, CFrame* pframe)
{
    if (argc <= 0)
        return MakeNoneRef();

    int bestIndex = 0;
    float bestValue = RefAsF32(argv[0]);

    for (int i = 1; i < argc; ++i)
    {
        const float value = RefAsF32(argv[i]);

        if (value > bestValue)
        {
            bestValue = value;
            bestIndex = i;
        }
    }

    // Preserve original winning type.
    return argv[bestIndex];
}

// =====================================================
// 95. RefOpMinimum
// =====================================================

CRef RefOpMinimum(int argc, CRef* argv, CFrame* pframe)
{
    if (argc <= 0)
        return MakeNoneRef();

    int bestIndex = 0;
    float bestValue = RefAsF32(argv[0]);

    for (int i = 1; i < argc; ++i)
    {
        const float value = RefAsF32(argv[i]);

        if (value < bestValue)
        {
            bestValue = value;
            bestIndex = i;
        }
    }

    // Preserve original winning type.
    return argv[bestIndex];
}

// =====================================================
// 96. RefOpModulo
// =====================================================

CRef RefOpModulo(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 2)
    {
        ret.SetS32(0);
        return ret;
    }

    const int lhs = RefAsS32(argv[0]);
    const int rhs = RefAsS32(argv[1]);

    if (rhs == 0)
    {
        // Original traps. This rewrite avoids crashing.
        ret.SetS32(0);
        return ret;
    }

    ret.SetS32(lhs % rhs);
    return ret;
}

// =====================================================
// 97. RefOpCurrentTime
// =====================================================

CRef RefOpCurrentTime(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;
    ret.SetF32(g_clock.t);
    return ret;
}

// =====================================================
// 98. RefOpScheduleCallback
// =====================================================

CRef RefOpScheduleCallback(int argc, CRef* argv, CFrame* pframe)
{
    // Original builds a callback pair and pushes it into the global
    // scheduled callback list binding.

    if (!g_psw || !g_psw->pframe || argc < 2)
    {
        return MakeVoidRef();
    }

    auto callback = PpairNew();
    auto timeNode = PpairNew();
    auto callNode = PpairNew();

    callback->m_ref.SetPair(timeNode);

    const float dtCallback = RefAsF32(argv[0]);

    timeNode->m_ref.SetF32(dtCallback);
    timeNode->m_ppairNext = callNode;

    auto procCall = PpairNew();
    auto quoteProc = PpairNew();
    auto procValue = PpairNew();

    callNode->m_ref.SetPair(procCall);

    procCall->m_ref.SetPair(quoteProc);

    quoteProc->m_ref.SetTag(TAGK_Quote);
    quoteProc->m_ppairNext = procValue;

    procValue->m_ref = argv[1];

    auto tail = procCall;

    for (int i = 2; i < argc; ++i)
    {
        auto argWrapper = PpairNew();
        auto quoteNode = PpairNew();
        auto valueNode = PpairNew();

        argWrapper->m_ref.SetPair(quoteNode);

        quoteNode->m_ref.SetTag(TAGK_Quote);
        quoteNode->m_ppairNext = valueNode;

        valueNode->m_ref = argv[i];

        tail->m_ppairNext = argWrapper;
        tail = argWrapper;
    }

    CRef oldList;

    if (g_psw->pframe->FFindBinding(g_psw->symidScheduledCallbackList, 0, &oldList))
    {
        if (oldList.m_tagk == TAGK_Pair)
            callback->m_ppairNext = oldList.m_ppair;
    }

    CRef newList;
    newList.SetPair(callback);

    g_psw->pframe->RefSetBinding(g_psw->symidScheduledCallbackList, &newList);

    return MakeVoidRef();
}

// =====================================================
// 99. RefOpDeferObjectUpdate
// =====================================================

CRef RefOpDeferObjectUpdate(int argc, CRef* argv, CFrame* pframe)
{
    CRef refRet;
    refRet.SetTag(TAGK_Void);

    if (argc < 2 || argv[0].m_tagk != TAGK_Basic || argv[0].m_pbasic == nullptr)
        return refRet;

    // Do not require CID_ALO yet. The original directly writes the field.
    ALO* plo = static_cast<ALO*>(argv[0].m_pbasic);
    plo->dtUpdatePause = RefAsF32(argv[1]);

    return refRet;
}

// =====================================================
// 100. RefOpAddO
// =====================================================

CRef RefOpAddO(int argc, CRef* argv, CFrame* pframe)
{
    // Original forwards to RefOpObjectOption(carg, aref, pframe, bifk).
    // Replace TAGK_AddO with your actual BIFK/enum value if named differently.
    return RefOpObjectOption(argc, argv, pframe, BIFK_AddO);
}

CRef RefOpObjectOption(int argc, CRef* argv, CFrame* pframe, BIFK bifk)
{
    CRef ret;

    const auto EmptyObjectResult = [bifk]()
        {
            CRef result;
            result.SetTag(bifk == BIFK_GetO ? TAGK_None : TAGK_Void);
            return result;
        };

    const int cargRequired = bifk == BIFK_GetO ? 2 : 3;

    if (argc < cargRequired)
    {
        ret.SetTag(TAGK_Err);
        return ret;
    }

    const int optid = RefAsOptid(argv[1]);

    auto ApplyObjectOption = [&](BASIC* pbasic) -> CRef
        {
            // A failed object lookup is a valid input to GetO.  RefGetOption
            // maps it to TAGK_None, matching splice's optional-object usage.
            if (bifk == BIFK_GetO)
            {
                return RefGetOption(pbasic, optid);
            }

            // Find-object represents a miss as a BASIC reference containing a
            // null pointer. Object-option operations treat that exactly like an
            // empty object list: reads produce None and writes are a no-op.
            if (pbasic == nullptr)
                return EmptyObjectResult();

            if (pbasic->pvtbasic == nullptr)
            {
                CRef error;
                error.SetTag(TAGK_Err);
                return error;
            }

            switch (bifk)
            {
            case BIFK_AddO:
                return RefAddOption(pbasic, optid, &argv[2]);

            case BIFK_EnsureO:
                return RefEnsureOption(pbasic, optid, &argv[2]);

            case BIFK_SetO:
                return RefSetOption(pbasic, optid, &argv[2]);

            default:
            {
                CRef error;
                error.SetTag(TAGK_Err);
                return error;
            }
            }
        };

    if (argv[0].m_tagk == TAGK_Basic)
    {
        BASIC* pbasicArg = argv[0].m_pbasic;
		CRef result = ApplyObjectOption(pbasicArg);
		return result;
    }

    if (argv[0].m_tagk == TAGK_Pair)
    {
        CPair* ppair = argv[0].m_ppair.get();
        int iobject = 0;
        bool fApplied = false;

        while (ppair != nullptr)
        {
            CRef& objectRef = ppair->m_ref;

            if (objectRef.m_tagk == TAGK_Basic)
            {
                BASIC* pbasicObject = objectRef.m_pbasic;

                if (pbasicObject == nullptr)
                {
                    // Missing optional objects are simply absent from the set.
                }
                else if (pbasicObject->pvtbasic == nullptr)
                {
                    std::printf("RefOpObjectOption: BASIC with null vtable at list index %d, object=%p, optid=%d\n", iobject, static_cast<void*>(pbasicObject), optid);
                }
				else
				{
					ret = ApplyObjectOption(pbasicObject);
					fApplied = true;
                }
            }
            else
            {
                std::printf("RefOpObjectOption: invalid list element at index %d, tag=%d, optid=%d\n", iobject, static_cast<int>(objectRef.m_tagk), optid);
            }

            ppair = ppair->m_ppairNext.get();
            ++iobject;
        }

        if (!fApplied)
            return EmptyObjectResult();

        if (bifk != BIFK_GetO)
            ret.SetTag(TAGK_Void);

        return ret;
    }

    if (argv[0].m_tagk == TAGK_None || argv[0].m_tagk == TAGK_Nil)
        return EmptyObjectResult();

    std::printf("RefOpObjectOption: expected object or object list, got tag=%d, optid=%d\n", static_cast<int>(argv[0].m_tagk), optid);

    ret.SetTag(TAGK_Err);
    return ret;
}

// =====================================================
// 101. RefOpEnsureO
// =====================================================

CRef RefOpEnsureO(int argc, CRef* argv, CFrame* pframe)
{
    return RefOpObjectOption(argc, argv, pframe, BIFK_EnsureO);
}

// =====================================================
// 102. RefOpSetO
// =====================================================

CRef RefOpSetO(int argc, CRef* argv, CFrame* pframe)
{
    return RefOpObjectOption(argc, argv, pframe, BIFK_SetO);
}

// =====================================================
// 103. RefOpGetO
// =====================================================

CRef RefOpGetO(int argc, CRef* argv, CFrame* pframe)
{
    return RefOpObjectOption(argc, argv, pframe, BIFK_GetO);
}

// =====================================================
// 104. RefOpFindObject
// =====================================================

CRef RefOpFindObject(int argc, CRef* argv, CFrame* pframe)
{
    if (g_psw == nullptr || argc < 2)
        return RefFromBasic(nullptr);

    const OID oid = static_cast<OID>(RefAsS32(argv[0]));
    LO* basis = RefAsLo(argv[1]);
    const GRFFSO grffso = basis != nullptr ? static_cast<GRFFSO>(0x101) : static_cast<GRFFSO>(0x105);
    LO* object = PloFindSwObject(g_psw, grffso, oid, basis);
    return RefFromBasic(static_cast<BASIC*>(object));
}

// =====================================================
// 105. RefOpFindObjects
// =====================================================

CRef RefOpFindObjects(int argc, CRef* argv, CFrame* pframe)
{
    if (g_psw == nullptr || argc < 2)
        return MakeNoneRef();

    LO* found[64]{};

    const OID oid = static_cast<OID>(RefAsS32(argv[0]));
    LO* basis = RefAsLo(argv[1]);
    const GRFFSO grffso = basis != nullptr ? static_cast<GRFFSO>(0x101) : static_cast<GRFFSO>(0x105);
    const int count = CploFindSwObjects(g_psw, grffso, oid, basis, 64, found);

    return RefFromPairList(PairListFromObjects(found, count));
}

// =====================================================
// 106. RefOpFindNearestObject
// =====================================================

CRef RefOpFindNearestObject(int argc, CRef* argv, CFrame* pframe)
{
    if (g_psw == nullptr || argc < 2)
        return RefFromBasic(nullptr);

    const OID oid = static_cast<OID>(RefAsS32(argv[0]));
    LO* basis = RefAsLo(argv[1]);
    const GRFFSO grffso = static_cast<GRFFSO>(0x104);
    LO* object = PloFindSwObject(g_psw, grffso, oid, basis);

    return RefFromBasic(static_cast<BASIC*>(object));
}

// =====================================================
// 107. RefOpFindNearestObjects
// =====================================================

CRef RefOpFindNearestObjects(int argc, CRef* argv, CFrame* pframe)
{
    if (g_psw == nullptr || argc < 2)
        return MakeNoneRef();

    LO* found[64]{};

    const OID oid = static_cast<OID>(RefAsS32(argv[0]));
    LO* basis = RefAsLo(argv[1]);
    const int count = CploFindSwObjects(g_psw, static_cast<GRFFSO>(0x104), oid, basis, 64, found);

    return RefFromPairList(PairListFromObjects(found, count));
}

// =====================================================
// 108. RefOpFindPlayerObject
// =====================================================

CRef RefOpFindPlayerObject(int argc, CRef* argv, CFrame* pframe)
{
    return RefFromBasic(reinterpret_cast<BASIC*>(PpoCur()));
}

// =====================================================
// 109. RefOpFindWorldObject
// =====================================================

CRef RefOpFindWorldObject(int argc, CRef* argv, CFrame* pframe)
{
    return RefFromBasic(reinterpret_cast<BASIC*>(g_psw));
}

// =====================================================
// 110. RefOpFindCameraObject
// =====================================================

CRef RefOpFindCameraObject(int argc, CRef* argv, CFrame* pframe)
{
    return RefFromBasic(reinterpret_cast<BASIC*>(g_pcm));
}

// =====================================================
// 111. RefOpFindClassObjects
// =====================================================

CRef RefOpFindClassObjects(int argc, CRef* argv, CFrame* pframe)
{
    if (g_psw == nullptr || argc < 2)
        return MakeNoneRef();

	LO* found[256]{};

    const CID cid = static_cast<CID>(RefAsS32(argv[0]));
    LO* basis = RefAsLo(argv[1]);
    const GRFFSO grffso = basis != nullptr ? static_cast<GRFFSO>(0x101) : static_cast<GRFFSO>(0x105);
	const int count = CploFindSwObjectsByClass(
		g_psw, grffso, cid, static_cast<ALO*>(basis), 256, found);

    return RefFromPairList(PairListFromObjects(found, count));
}

// =====================================================
// 112. RefOpFindObjectsInBoundingBox
// =====================================================

CRef RefOpFindObjectsInBoundingBox(int argc, CRef* argv, CFrame* pframe)
{
    if (!g_psw || argc < 2)
        return MakeNoneRef();

    const glm::vec3 a = RefAsVector(argv[0]);
    const glm::vec3 b = RefAsVector(argv[1]);

    std::vector<SO*> found;

    IntersectSwBoundingBox(g_psw, nullptr, &a, &b, nullptr, nullptr, found);

    return RefFromPairList(PairListFromSoVector(found));
}

// =====================================================
// 113. RefOpFindObjectsInBoundingSphere
// =====================================================

CRef RefOpFindObjectsInBoundingSphere(int argc, CRef* argv, CFrame* pframe)
{
    if (g_psw == nullptr || argv == nullptr || argc < 2)
        return MakeNoneRef();

    const glm::vec3 center = RefAsVector(argv[0]);
    const float radius = RefAsF32(argv[1]);

    std::vector<SO*> found;
    IntersectSwBoundingSphere(g_psw, nullptr, &center, radius, nullptr, nullptr, found);

    return RefFromPairList(PairListFromSoVector(found));
}

CRef RefOpHitTestObjectsImpl(BIFK bifk, int carg, CRef* aref, CFrame* pframe)
{
    (void)pframe;

    CRef refRet;
    if (aref == nullptr || carg < 3 || aref[0].m_tagk != TAGK_Pair || !aref[0].m_ppair)
    {
        refRet.SetTag(TAGK_None);
        return refRet;
    }

    glm::vec3 posStart = RefAsVector(aref[1]);
    glm::vec3 posEnd = RefAsVector(aref[2]);
    std::shared_ptr<CPair> resultHead;
    std::shared_ptr<CPair> resultTail;

    for (auto objectNode = aref[0].m_ppair; objectNode; objectNode = objectNode->m_ppairNext)
    {
        SO* pso = RefAsSo(objectNode->m_ref);
        if (pso == nullptr)
            continue;

        LSG lsg{};
        std::vector<SO*> objects{ pso };
        if (PsoHitTestLineObjects(0, &posStart, &posEnd, objects, &lsg) == nullptr)
            continue;

        CRef refObject;
        refObject.SetBasic(static_cast<BASIC*>(pso));

        if (bifk == BIFK_HitTestObjectsFirst)
        {
            auto objectResult = MakePairNode(refObject);
            CRef refPosition;
            refPosition.SetVector(lsg.apos[1]);
            objectResult->m_ppairNext = MakePairNode(refPosition);
            refRet.SetPair(objectResult);
            return refRet;
        }

        auto hitResult = MakePairNode(refObject);
        CRef refPos0;
        refPos0.SetVector(lsg.apos[0]);
        hitResult->m_ppairNext = MakePairNode(refPos0);

        CRef refPos1;
        refPos1.SetVector(lsg.apos[1]);
        hitResult->m_ppairNext->m_ppairNext = MakePairNode(refPos1);

        auto resultNode = PpairNew();
        resultNode->m_ref.SetPair(hitResult);
        if (!resultHead)
            resultHead = resultNode;
        else
            resultTail->m_ppairNext = resultNode;
        resultTail = resultNode;
    }

    if (resultHead)
        refRet.SetPair(resultHead);
    else
        refRet.SetTag(TAGK_None);
    return refRet;
}

// =====================================================
// 114. RefOpHitTestObjects
// =====================================================

CRef RefOpHitTestObjects(int argc, CRef* argv, CFrame* pframe)
{
    return RefOpHitTestObjectsImpl(BIFK_HitTestObjects, argc, argv, pframe);
}

// =====================================================
// 115. RefOpHitTestObjectsFirst
// =====================================================

CRef RefOpHitTestObjectsFirst(int argc, CRef* argv, CFrame* pframe)
{
    return RefOpHitTestObjectsImpl(BIFK_HitTestObjectsFirst, argc, argv, pframe);
}

// =====================================================
// 116. RefOpConvertObjectPosition
// =====================================================

CRef RefOpConvertObjectPosition(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 3)
    {
        ret.SetVector(glm::vec3{});
        return ret;
    }

    glm::vec3 out{};
    glm::vec3 posFrom = RefAsVector(argv[2]);

    ConvertAloPos(RefAsAlo(argv[0]), RefAsAlo(argv[1]), &posFrom, &out);

    ret.SetVector(out);
    return ret;
}

// =====================================================
// 117. RefOpConvertObjectVector
// =====================================================

CRef RefOpConvertObjectVector(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 3)
    {
        ret.SetVector(glm::vec3{});
        return ret;
    }

    glm::vec3 out{};
    glm::vec3 pvecFrom = RefAsVector(argv[2]);
    ConvertAloVec(RefAsAlo(argv[0]), RefAsAlo(argv[1]), &pvecFrom, &out);

    ret.SetVector(out);
    return ret;
}

// =====================================================
// 118. RefOpConvertObjectMatrix
// =====================================================

CRef RefOpConvertObjectMatrix(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 3)
    {
        ret.SetMatrix(glm::mat4{ 1.0f });
        return ret;
    }

    glm::mat3 out{ 1.0f };
    glm::mat3 pmatFrom = RefAsMatrix(argv[2]);
    ConvertAloMat(RefAsAlo(argv[0]), RefAsAlo(argv[1]), &pmatFrom, &out);

    ret.SetMatrix(out);
    return ret;
}

// =====================================================
// 119. RefOpNearClipCenter
// =====================================================

CRef RefOpNearClipCenter(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (g_pcm == nullptr)
    {
        ret.SetVector(glm::vec3(0.0f));
        return ret;
    }

    ret.SetVector(g_pcm->pos + g_pcm->mat[0] * g_pcm->sNearClip);
    return ret;
}

// =====================================================
// 120. RefOpStartSound
// =====================================================

CRef RefOpStartSound(int argc, CRef* argv, CFrame* pframe)
{
    CRef ret;

    if (argc < 1)
    {
        ret.SetS32(0);
        return ret;
    }

    AMB* amb = nullptr;

    const SFXID sfxid = static_cast<SFXID>(RefAsS32(argv[0]));
    ALO *source = HasArg(argc, 1) ? RefAsAlo(argv[1]) : nullptr;

    StartSound(sfxid, &amb, source, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

    ret.SetS32(static_cast<s32>(reinterpret_cast<intptr_t>(amb)));

    if (amb)
        DropPamb(&amb);

    return ret;
}

// =====================================================
// 121. RefOpStopSound
// =====================================================

CRef RefOpStopSound(int argc, CRef* argv, CFrame* pframe)
{
    if (argc >= 1)
        StopSound(RefAsAmb(argv[0]), 0);

    return MakeVoidRef();
}

// =====================================================
// 122. RefOpStartRumble
// =====================================================

CRef RefOpStartRumble(int argc, CRef* argv, CFrame* pframe)
{
    if (argc >= 2)
    {
        const RUMK rumk = static_cast<RUMK>(RefAsS32(argv[0]));
        const float strength = RefAsF32(argv[1]);
        //TriggerJoyRumbleRumk(&g_joy, rumk, strength);
    }

    return MakeVoidRef();
}

// =====================================================
// 123. RefOpEmitSmokeCloud
// =====================================================

CRef RefOpEmitSmokeCloud(int argc, CRef* argv, CFrame* pframe)
{
    if (argc >= 2)
    {
        glm::vec3 pos = RefAsVector(argv[0]);
        StandardSmokeCloud(&pos, RefAsF32(argv[1]));
    }

    return MakeVoidRef();
}

// =====================================================
// 124. RefOpPredictAnimationEffect
// =====================================================

CRef RefOpPredictAnimationEffect(int argc, CRef* argv, CFrame* pframe)
{
    if (argc < 7)
        return MakeNoneRef();

    ASEG* paseg = RefAsAseg(argv[0]);
    const float t = RefAsF32(argv[1]);
    const float svt = RefAsF32(argv[2]);
    ALO* paloLeaf = RefAsAlo(argv[3]);
    ALO* paloBasis = RefAsAlo(argv[4]);
    const int cpaloPredict = RefAsS32(argv[5]);
    const APEK apek = static_cast<APEK>(RefAsS32(argv[6]));

    if (paseg == nullptr || cpaloPredict < 0)
        return MakeNoneRef();

    std::vector<glm::vec3> apos;
    std::vector<glm::mat3> amat;
    std::vector<glm::vec3> av;
    std::vector<glm::vec3> aw;

    if ((apek & APEK_Pos) != 0)
        apos.resize(cpaloPredict);

    if ((apek & APEK_Rot) != 0)
        amat.resize(cpaloPredict);

    if ((apek & APEK_Vel) != 0)
        av.resize(cpaloPredict);

    if ((apek & APEK_Angvel) != 0)
        aw.resize(cpaloPredict);

    PredictAsegEffect(paseg, t, svt, paloLeaf, paloBasis, cpaloPredict, apos.empty() ? nullptr : apos.data(), amat.empty() ? nullptr : amat.data(), av.empty() ? nullptr : av.data(), aw.empty() ? nullptr : aw.data());

    std::shared_ptr<CPair> ppairHead;
    std::shared_ptr<CPair> ppairTail;

    auto AppendOuterList = [&](const std::shared_ptr<CPair>& ppairSub)
    {
        std::shared_ptr<CPair> ppair = PpairNew();

        if (ppairSub != nullptr)
            ppair->m_ref.SetPair(ppairSub);
        else
            ppair->m_ref.SetTag(TAGK_None);

        if (ppairHead == nullptr)
            ppairHead = ppair;
        else
            ppairTail->m_ppairNext = ppair;

        ppairTail = ppair;
    };

    auto BuildVectorList = [&](const std::vector<glm::vec3>& values)
    {
        std::shared_ptr<CPair> head;
        std::shared_ptr<CPair> tail;

        for (const glm::vec3& value : values)
        {
            std::shared_ptr<CPair> pair = PpairNew();
            pair->m_ref.SetVector(value);

            if (head == nullptr)
                head = pair;
            else
                tail->m_ppairNext = pair;

            tail = pair;
        }

        return head;
    };

    auto BuildMatrixList = [&](const std::vector<glm::mat3>& values)
    {
        std::shared_ptr<CPair> head;
        std::shared_ptr<CPair> tail;

        for (const glm::mat3& value : values)
        {
            std::shared_ptr<CPair> pair = PpairNew();
            pair->m_ref.SetMatrix(value);

            if (head == nullptr)
                head = pair;
            else
                tail->m_ppairNext = pair;

            tail = pair;
        }

        return head;
    };

    if ((apek & APEK_Pos) != 0)
        AppendOuterList(BuildVectorList(apos));

    if ((apek & APEK_Rot) != 0)
        AppendOuterList(BuildMatrixList(amat));

    if ((apek & APEK_Vel) != 0)
        AppendOuterList(BuildVectorList(av));

    if ((apek & APEK_Angvel) != 0)
        AppendOuterList(BuildVectorList(aw));

    return RefFromPairList(ppairHead);
}
// =====================================================
// 125. RefOpSetMusicRegister
// =====================================================

CRef RefOpSetMusicRegister(int argc, CRef* argv, CFrame* pframe)
{
    if (argc >= 2)
        SetAMRegister(RefAsS32(argv[0]), static_cast<byte>(RefAsS32(argv[1])));

    return MakeVoidRef();
}

#include "spliceobj.h"
