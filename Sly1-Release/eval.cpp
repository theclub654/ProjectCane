#include "eval.h"
#include "splicemap.h"
#include "proc.h"
#include "lo.h"
#include "spaprops.h"
#include <cstdio>
#include <cstring>

namespace
{
}

CRef RefEvalSymbol(CPair* ppair, CFrame* pframe)
{
    CRef ref;

    if (!ppair || !pframe)
    {
        ref.SetTag(TAGK_Err);
        return ref;
    }

    SYMID symid = ppair->m_ref.GetSymid();

    if (!pframe->FFindBinding(symid, 1, &ref))
    {
        //std::printf("RefEvalSymbol: binding not found: symid=%d", static_cast<int>(symid));

        /*if (g_psw != nullptr && static_cast<size_t>(symid) < g_psw->mpsymidachz.size())
            std::printf(" name=%s", g_psw->mpsymidachz[symid].c_str());*/

        //std::printf("\n");

        ref.SetTag(TAGK_Err);
        return ref;
    }

    if (ref.m_tagk == TAGK_Basic)
    {
        const char* name = "<unknown>";

        if (g_psw != nullptr && static_cast<size_t>(symid) < g_psw->mpsymidachz.size())
            name = g_psw->mpsymidachz[symid].c_str();

        //std::printf("RefEvalSymbol: symid=%u name=%s basic=%p pvtbasic=%p\n", static_cast<unsigned>(symid), name, static_cast<void*>(ref.m_pbasic), ref.m_pbasic != nullptr ? static_cast<void*>(ref.m_pbasic->pvtbasic) : nullptr);
    }

    return ref;
}

CRef RefEval(CPair* ppair, CFrame* pframe)
{
    CRef refRet;

    if (ppair == nullptr)
    {
        refRet.SetTag(TAGK_Nil);
        return refRet;
    }

    switch (ppair->m_ref.m_tagk)
    {
        // Self-evaluating values
        case TAGK_Nil:
        case TAGK_None:
        case TAGK_S32:
        case TAGK_F32:
        case TAGK_Vector:
        case TAGK_Matrix:
        case TAGK_Clq:
        case TAGK_Lm:
        case TAGK_Smp:
        case TAGK_Bool:
        case TAGK_Bifk:
        case TAGK_Basic:
        case TAGK_Method:
        case TAGK_Proc:
        refRet = ppair->m_ref;
        break;

        case TAGK_Symid:
        refRet = RefEvalSymbol(ppair, pframe);
        break;

        case TAGK_Pair:
        {
            CPair* list = ppair->m_ref.m_ppair.get();

            if (list == nullptr)
            {
                refRet.SetTag(TAGK_Nil);
                break;
            }

            switch (list->m_ref.m_tagk)
            {
                case TAGK_Set:
                refRet = RefEvalSet(ppair, pframe);
                break;

                case TAGK_Define:
                refRet = RefEvalDefine(ppair, pframe);
                break;

                case TAGK_Assert:
                refRet = RefEvalAssert(ppair, pframe);
                break;

                case TAGK_If:
                refRet = RefEvalIf(ppair, pframe);
                break;

                case TAGK_Or:
                refRet = RefEvalOr(ppair, pframe);
                break;

                case TAGK_And:
                refRet = RefEvalAnd(ppair, pframe);
                break;

                case TAGK_Cond:
                refRet = RefEvalCond(ppair, pframe);
                break;

                case TAGK_Case:
                refRet = RefEvalCase(ppair, pframe);
                break;

                case TAGK_Let:
                refRet = RefEvalLet(ppair, pframe);
                break;

                case TAGK_While:
                refRet = RefEvalWhile(ppair, pframe);
                break;

                case TAGK_Lambda:
                refRet = RefEvalLambda(ppair, pframe);
                break;

                case TAGK_Begin:
                refRet = RefEvalBegin(ppair, pframe);
                break;

                case TAGK_Import:
                refRet = RefEvalImport(ppair, pframe);
                break;

                case TAGK_Quote:
                {
                    CPair* quoted = list->m_ppairNext.get();

                    if (quoted)
                        refRet = quoted->m_ref;
                    else
                        refRet.SetTag(TAGK_Nil);

                    break;
                }

                default:
                refRet = RefEvalApply(ppair, pframe);
                break;
            }

            break;
        }

        default:
        refRet.SetTag(TAGK_Void);
        break;
    }

    return refRet;
}

CRef RefEvalSet(CPair* ppair, CFrame* pframe)
{
    CRef ret;

    CPair* list = ppair->m_ref.m_ppair.get();
    if (!list)
    {
        ret.SetTag(TAGK_Void);
        return ret;
    }

    // list = (set symbol expression)
    CPair* symbolPair = list->m_ppairNext.get();
    CPair* valuePair = symbolPair ? symbolPair->m_ppairNext.get() : nullptr;

    if (!symbolPair || !valuePair)
    {
        ret.SetTag(TAGK_Void);
        return ret;
    }

    CRef value = RefEval(valuePair, pframe);

    if (symbolPair->m_ref.m_tagk != TAGK_Symid)
    {
        CPair* const target = symbolPair->m_ref.m_tagk == TAGK_Pair
            ? symbolPair->m_ref.m_ppair.get()
            : nullptr;
        CPair* const target1 = target != nullptr ? target->m_ppairNext.get() : nullptr;
        CPair* const target2 = target1 != nullptr ? target1->m_ppairNext.get() : nullptr;

        std::printf(
            "EVAL SET COMPLEX targetTag=%d headTag=%d headSym=%u arg1Tag=%d arg1Sym=%u "
            "arg2Tag=%d arg2S32=%d valueTag=%d valueVector=(%.6f %.6f %.6f)\n",
            static_cast<int>(symbolPair->m_ref.m_tagk),
            target != nullptr ? static_cast<int>(target->m_ref.m_tagk) : -1,
            target != nullptr ? static_cast<unsigned>(target->m_ref.m_symid) : 0u,
            target1 != nullptr ? static_cast<int>(target1->m_ref.m_tagk) : -1,
            target1 != nullptr ? static_cast<unsigned>(target1->m_ref.m_symid) : 0u,
            target2 != nullptr ? static_cast<int>(target2->m_ref.m_tagk) : -1,
            target2 != nullptr ? target2->m_ref.m_n : 0,
            static_cast<int>(value.m_tagk),
            value.m_tagk == TAGK_Vector ? value.m_vector.x : 0.0f,
            value.m_tagk == TAGK_Vector ? value.m_vector.y : 0.0f,
            value.m_tagk == TAGK_Vector ? value.m_vector.z : 0.0f);

    }

    SYMID symid = symbolPair->m_ref.m_symid;

    if (value.m_tagk == TAGK_Vector || value.m_tagk == TAGK_Matrix)
    {
        const char* name = "<unknown>";
        if (g_psw != nullptr && static_cast<std::size_t>(symid) < g_psw->mpsymidachz.size())
            name = g_psw->mpsymidachz[symid].c_str();

        CRef existing;
        const int found = pframe->FFindBinding(symid, 1, &existing);
        /*std::printf(
            "EVAL SET VALUE symid=%u name=%s valueTag=%d vector=(%.6f %.6f %.6f) "
            "bindingFound=%d bindingTag=%d bindingBasic=%p\n",
            static_cast<unsigned>(symid), name, static_cast<int>(value.m_tagk),
            value.m_tagk == TAGK_Vector ? value.m_vector.x : 0.0f,
            value.m_tagk == TAGK_Vector ? value.m_vector.y : 0.0f,
            value.m_tagk == TAGK_Vector ? value.m_vector.z : 0.0f,
            found, found != 0 ? static_cast<int>(existing.m_tagk) : -1,
            found != 0 && existing.m_tagk == TAGK_Basic
                ? static_cast<void*>(existing.m_pbasic)
                : nullptr);*/
    }

    pframe->RefSetBinding(symid, &value);

    ret = value;
    return ret;
}

CRef RefEvalDefine(CPair* ppair, CFrame* pframe)
{
    CRef ret;

    CPair* list = ppair->m_ref.m_ppair.get();
    if (!list)
    {
        ret.SetTag(TAGK_Void);
        return ret;
    }

    // list = (define symbol expression)
    CPair* symbolPair = list->m_ppairNext.get();
    CPair* valuePair = symbolPair ? symbolPair->m_ppairNext.get() : nullptr;

    if (!symbolPair || !valuePair)
    {
        ret.SetTag(TAGK_Void);
        return ret;
    }

    SYMID symid = symbolPair->m_ref.m_symid;

    CRef value = RefEval(valuePair, pframe);

    pframe->RefAddBinding(symid, &value);

    ret = value;
    return ret;
}

CRef RefEvalAssert(CPair* ppair, CFrame* pframe)
{
    CRef ret;

    CPair* list = ppair->m_ref.m_ppair.get();
    if (!list)
    {
        ret.SetTag(TAGK_Void);
        return ret;
    }

    // list = (assert expression)
    CPair* exprPair = list->m_ppairNext.get();

    if (exprPair)
    {
        CRef result = RefEval(exprPair, pframe);

        bool asserted = true;
        switch (result.m_tagk)
        {
        case TAGK_Bool:
            asserted = result.m_bool != 0;
            break;
        case TAGK_S32:
            asserted = result.m_n != 0;
            break;
        case TAGK_F32:
            asserted = result.m_g != 0.0f;
            break;
        default:
            break;
        }

        if (!asserted)
        {
            printf("Splice assert: ");
            //RefOpPrint(exprPair);
        }
    }

    ret.SetTag(TAGK_Void);
    return ret;
}

CRef RefEvalIf(CPair* ppair, CFrame* pframe)
{
    CRef ret;

    CPair* list = ppair->m_ref.m_ppair.get();
    if (!list)
    {
        ret.SetTag(TAGK_Void);
        return ret;
    }

    CPair* condPair = list->m_ppairNext.get();
    CPair* thenPair = condPair ? condPair->m_ppairNext.get() : nullptr;
    CPair* elsePair = thenPair ? thenPair->m_ppairNext.get() : nullptr;

    if (!condPair || !thenPair)
    {
        ret.SetTag(TAGK_Void);
        return ret;
    }

    CRef cond = RefEval(condPair, pframe);

    // Splice uses Scheme-style truth: only Bool false is false.  The other
    // evaluator forms (and/or/cond/while) already follow this rule.  Testing
    // the raw union payload against 1 incorrectly rejected objects, numbers,
    // vectors, and other truthy values.
    if (cond.m_tagk != TAGK_Bool || cond.m_bool != 0)
        ret = RefEval(thenPair, pframe);
    else if (elsePair != nullptr)
        ret = RefEval(elsePair, pframe);
    else
        ret.SetTag(TAGK_Void);

    return ret;
}

CRef RefEvalOr(CPair* ppair, CFrame* pframe)
{
    CRef ret;

    CPair* list = ppair->m_ref.m_ppair.get();
    if (!list)
    {
        ret.SetBool(0);
        return ret;
    }

    // list = (or expr1 expr2 expr3 ...)
    CPair* expr = list->m_ppairNext.get();

    while (expr != nullptr)
    {
        ret = RefEval(expr, pframe);

        // Original behavior:
        // return immediately if result is not Bool false.
        if (ret.m_tagk != TAGK_Bool || ret.m_bool != 0)
            return ret;

        expr = expr->m_ppairNext.get();
    }

    ret.SetBool(0);
    return ret;
}

CRef RefEvalAnd(CPair* ppair, CFrame* pframe)
{
    CRef ret;
    ret.SetBool(1);

    CPair* list = ppair->m_ref.m_ppair.get();
    if (!list)
        return ret;

    // list = (and expr1 expr2 expr3 ...)
    CPair* expr = list->m_ppairNext.get();

    while (expr != nullptr)
    {
        ret = RefEval(expr, pframe);

        // Original behavior:
        // only Bool false stops evaluation.
        if (ret.m_tagk == TAGK_Bool && ret.m_bool == 0)
            return ret;

        expr = expr->m_ppairNext.get();
    }

    return ret;
}

CRef RefEvalCond(CPair* ppair, CFrame* pframe)
{
    CRef ret;
    CRef test;

    ret.SetTag(TAGK_Void);

    CPair* list = ppair->m_ref.m_ppair.get();
    if (!list)
        return ret;

    // list = (cond clause1 clause2 clause3 ...)
    CPair* clauseNode = list->m_ppairNext.get();

    while (clauseNode != nullptr)
    {
        CPair* clause = clauseNode->m_ref.m_ppair.get();
        if (!clause)
        {
            clauseNode = clauseNode->m_ppairNext.get();
            continue;
        }

        CPair* testExpr = clause;
        CPair* bodyExpr = clause->m_ppairNext.get();

        bool passed = false;

        // TAGK 10 looks like "else"
        if (testExpr->m_ref.m_tagk == TAGK_Else)
        {
            passed = true;
        }
        else
        {
            test = RefEval(testExpr, pframe);

            if (test.m_tagk == TAGK_Bool)
                passed = (test.m_bool != 0);
            else
                passed = true;
        }

        if (passed)
        {
            if (bodyExpr == nullptr)
            {
                ret = test;
            }
            else
            {
                while (bodyExpr != nullptr)
                {
                    ret = RefEval(bodyExpr, pframe);
                    bodyExpr = bodyExpr->m_ppairNext.get();
                }
            }

            return ret;
        }

        clauseNode = clauseNode->m_ppairNext.get();
    }

    return ret;
}

CRef RefEvalCase(CPair* ppair, CFrame* pframe)
{
    CRef ret;
    ret.SetTag(TAGK_Void);

    CPair* list = ppair->m_ref.m_ppair.get();
    if (!list)
        return ret;

    // list = (case expr clause1 clause2 ...)
    CPair* exprPair = list->m_ppairNext.get();
    if (!exprPair)
        return ret;

    CRef caseValue = RefEval(exprPair, pframe);

    CPair* clauseNode = exprPair->m_ppairNext.get();

    while (clauseNode != nullptr)
    {
        CPair* clause = clauseNode->m_ref.m_ppair.get();
        if (!clause)
        {
            clauseNode = clauseNode->m_ppairNext.get();
            continue;
        }

        CPair* matchList = clause;
        CPair* bodyExpr = clause->m_ppairNext.get();

        bool matched = false;

        // TAGK 10 appears to be else
        if (matchList->m_ref.m_tagk == TAGK_Else)
        {
            matched = true;
        }
        else
        {
            // matchList is itself a list of possible values
            CPair* valueNode = matchList->m_ref.m_ppair.get();

            while (valueNode != nullptr)
            {
                CRef args[2];
                args[0] = caseValue;
                args[1] = valueNode->m_ref;

                CRef eq = RefOpEqv(2, args, pframe);

                if (eq.m_tagk == TAGK_Bool && eq.m_bool != 0)
                {
                    matched = true;
                    break;
                }

                valueNode = valueNode->m_ppairNext.get();
            }
        }

        if (matched)
        {
            while (bodyExpr != nullptr)
            {
                ret = RefEval(bodyExpr, pframe);
                bodyExpr = bodyExpr->m_ppairNext.get();
            }

            return ret;
        }

        clauseNode = clauseNode->m_ppairNext.get();
    }

    return ret;
}

CRef RefEvalLet(CPair* ppair, CFrame* pframe)
{
    CRef ret;

    if (!ppair || !pframe)
    {
        ret.SetTag(TAGK_Void);
        return ret;
    }

    CPair* list = ppair->m_ref.m_ppair.get();

    if (!list)
    {
        ret.SetTag(TAGK_Void);
        return ret;
    }

    CPair* bindingsNode = list->m_ppairNext.get();
    CPair* body = bindingsNode ? bindingsNode->m_ppairNext.get() : nullptr;

    std::shared_ptr<CFrame> letFrameOwner;
    CFrame* evalFrame = pframe;

    if (bindingsNode && bindingsNode->m_ref.m_tagk != TAGK_Nil)
    {
        letFrameOwner = PframeNew();
        CFrame* letFrame = letFrameOwner.get();

        letFrame->SetSingleParent(pframe);

        CPair* bindings = bindingsNode->m_ref.m_ppair.get();

        while (bindings)
        {
            CPair* binding = bindings->m_ref.m_ppair.get();

            if (binding && binding->m_ppairNext)
            {
                CRef value = RefEval(binding->m_ppairNext.get(), pframe);
                letFrame->RefAddBinding(binding->m_ref.GetSymid(), &value);
            }

            bindings = bindings->m_ppairNext.get();
        }

        evalFrame = letFrame;
    }

    while (body)
    {
        ret = RefEval(body, evalFrame);
        body = body->m_ppairNext.get();
    }

    return ret;
}

CRef RefEvalWhile(CPair* ppair, CFrame* pframe)
{
    CRef ret;

    CPair* list = ppair->m_ref.m_ppair.get();
    if (!list)
    {
        ret.SetTag(TAGK_Void);
        return ret;
    }

    // list = (while condition body)
    CPair* condPair = list->m_ppairNext.get();
    CPair* bodyPair = condPair ? condPair->m_ppairNext.get() : nullptr;

    while (condPair != nullptr)
    {
        ret = RefEval(condPair, pframe);

        // Stop only on Bool false.
        if (ret.m_tagk == TAGK_Bool && ret.m_bool == 0)
            break;

        // A while form may contain more than one body expression.
        for (CPair* body = bodyPair; body != nullptr; body = body->m_ppairNext.get())
            ret = RefEval(body, pframe);
    }

    ret.SetTag(TAGK_Void);
    return ret;
}

CRef RefEvalLambda(CPair* ppair, CFrame* pframe)
{
    CRef ret;

    CPair* list = ppair->m_ref.m_ppair.get();
    if (!list)
    {
        ret.SetTag(TAGK_Void);
        return ret;
    }

    // list = (lambda formals body...)
    CPair* formalsNode = list->m_ppairNext.get();   
    CPair* bodyNode = formalsNode ? formalsNode->m_ppairNext.get() : nullptr;

    std::shared_ptr <CProc> proc = PprocNew();

    // A procedure can outlive the evaluation that creates it (for example when
    // stored in an object's sidebag), so retain its lexical environment.
    proc->m_pframeOwner = pframe->shared_from_this();
    proc->m_pframe = proc->m_pframeOwner.get();

    if (formalsNode && formalsNode->m_ref.m_tagk != TAGK_Nil)
        proc->m_ppairFormals = formalsNode->m_ref.m_ppair.get();
    else
        proc->m_ppairFormals = nullptr;

    proc->m_ppairCodeExpr = bodyNode;

    int requiredCount = 0;
    int isVarArg = 0;

    for (CPair* formal = proc->m_ppairFormals; formal != nullptr; formal = formal->m_ppairNext.get())
    {
        if (formal->m_ref.m_tagk == TAGK_Pipe)
        {
            isVarArg = 1;
            break;
        }

        requiredCount++;
    }

    proc->m_crefReq = requiredCount;
    proc->m_fVarArg = isVarArg;

    ret.SetProc(proc);
    return ret;
}

CRef RefEvalBegin(CPair* ppair, CFrame* pframe)
{
    CRef ret;
    ret.SetTag(TAGK_Void);

    CPair* list = ppair->m_ref.m_ppair.get();
    if (!list)
        return ret;

    // list = (begin expr1 expr2 expr3 ...)
    CPair* expr = list->m_ppairNext.get();

    while (expr != nullptr)
    {
        ret = RefEval(expr, pframe);
        expr = expr->m_ppairNext.get();
    }

    return ret;
}

CRef RefEvalLambdaBody(CPair* ppair, CFrame* pframe)
{
    CRef ret;
    ret.SetTag(TAGK_Void);

    CPair* expr = ppair;

    while (expr != nullptr)
    {
        ret = RefEval(expr, pframe);
        expr = expr->m_ppairNext.get();
    }

    return ret;
}

CRef RefEvalImport(CPair* ppair, CFrame* pframe)
{
    CRef ret;

    CPair* list = ppair->m_ref.m_ppair.get();
    if (!list)
    {
        ret.SetTag(TAGK_Void);
        return ret;
    }

    // list = (import spliceId)
    CPair* splicePair = list->m_ppairNext.get();
    if (!splicePair)
    {
        ret.SetTag(TAGK_Void);
        return ret;
    }

    int isplice = splicePair->m_ref.m_n;

    std::shared_ptr <CFrame> importedFrame = PframeFromIsplice(isplice, g_psw);

    if (importedFrame == nullptr)
    {
        ret = RefEvalModule(g_psw, isplice);
        importedFrame = PframeFromIsplice(isplice, g_psw);
    }

    if (importedFrame != nullptr)
    {
        pframe->AddParent(importedFrame.get());

    }

    ret.SetTag(TAGK_Void);
    return ret;
}

CRef RefEvalApply(CPair* ppair, CFrame* pframe)
{
    CRef ret;

    if (!ppair || !pframe || ppair->m_ref.m_tagk != TAGK_Pair)
    {
        ret.SetTag(TAGK_Err);
        return ret;
    }

    CPair* list = ppair->m_ref.m_ppair.get();

    if (!list)
    {
        ret.SetTag(TAGK_Err);
        return ret;
    }

    CRef refOperator = RefEval(list, pframe);

    int requiredCount = 0;
    int hasVarArg = 0;

    BIF* pbif = nullptr;
    std::shared_ptr<CProc> proc;
    std::shared_ptr<CMethod> method;

    if (refOperator.m_tagk == TAGK_Bifk)
    {
        const int bifIndex = static_cast<int>(refOperator.m_bifk);
        if (bifIndex < 0 || bifIndex >= 125)
        {
            ret.SetTag(TAGK_Err);
            return ret;
        }

        pbif = &g_mpbifkbif[bifIndex];
        requiredCount = pbif->crefReq;
        hasVarArg = pbif->fVarArg;

    }
    else if (refOperator.m_tagk == TAGK_Method)
    {
        method = refOperator.m_pmethod;

        if (!method)
        {
            //std::printf("RefEvalApply: null method\n");
            ret.SetTag(TAGK_Err);
            return ret;
        }

        requiredCount = method->m_crefReq;
        hasVarArg = 0;

    }
    else if (refOperator.m_tagk == TAGK_Proc)
    {
        proc = refOperator.m_pproc;

        if (!proc)
        {
            //std::printf("RefEvalApply: TAGK_Proc has null m_pproc\n");
            ret.SetTag(TAGK_Err);
            return ret;
        }

        requiredCount = proc->m_crefReq;
        hasVarArg = proc->m_fVarArg;

    }
    else
    {
        /*std::printf(
            "RefEvalApply: operator is not callable; resultTag=%d sourceTag=%d",
            static_cast<int>(refOperator.m_tagk),
            static_cast<int>(list->m_ref.m_tagk));*/

        if (list->m_ref.m_tagk == TAGK_Symid)
        {
            SYMID symid = list->m_ref.GetSymid();
            int symidIndex = static_cast<int>(symid);

            /*std::printf(" symid=%d", symidIndex);

            if (g_psw &&
                symidIndex >= 0 &&
                static_cast<size_t>(symidIndex) <
                g_psw->mpsymidachz.size())
            {
                std::printf(
                    " name=%s",
                    g_psw->mpsymidachz[symidIndex].c_str());
            }*/
        }

        //std::printf("\n");

        ret.SetTag(TAGK_Err);
        return ret;
    }

    if (requiredCount < 0 || requiredCount > 32)
    {
        /*std::printf(
            "RefEvalApply: invalid requiredCount=%d\n",
            requiredCount);*/

        ret.SetTag(TAGK_Err);
        return ret;
    }

    CRef operands[32]{};

    int operandSlotCount = 0;
    int argumentCount = 0;

    CPair* arg = list->m_ppairNext.get();
    for (int i = 0; i < requiredCount; ++i)
    {
        if (!arg || operandSlotCount >= 32)
        {
           /* std::printf(
                "RefEvalApply: missing required argument %d of %d\n",
                i + 1,
                requiredCount);*/

            ret.SetTag(TAGK_Err);
            return ret;
        }

        operands[operandSlotCount] = RefEval(arg, pframe);

        operandSlotCount++;
        argumentCount++;

        arg = arg->m_ppairNext.get();
    }

    if (hasVarArg)
    {
        // Native BIFs receive a flat CRef array and the real argument count.
        // Lambda procedures, on the other hand, bind their rest parameter to
        // one list.  Packing BIF varargs into a TAGK_Pair made arithmetic such
        // as (* 6.283 0) silently ignore the zero and return 6.283.
        if (refOperator.m_tagk == TAGK_Bifk)
        {
            while (arg)
            {
                if (operandSlotCount >= 32)
                {
                    ret.SetTag(TAGK_Err);
                    return ret;
                }

                operands[operandSlotCount++] = RefEval(arg, pframe);
                argumentCount++;
                arg = arg->m_ppairNext.get();
            }
        }
        else
        {
            if (operandSlotCount >= 32)
            {
                ret.SetTag(TAGK_Err);
                return ret;
            }

            std::shared_ptr<CPair> head;
            std::shared_ptr<CPair> tail;

            while (arg)
            {
                auto node = std::make_shared<CPair>();
                node->m_ref = RefEval(arg, pframe);

                if (head)
                    tail->m_ppairNext = node;
                else
                    head = node;

                tail = node;
                argumentCount++;
                arg = arg->m_ppairNext.get();
            }

            if (head)
                operands[operandSlotCount].SetPair(head);
            else
                operands[operandSlotCount].SetTag(TAGK_None);

            operandSlotCount++;
        }
    }

    if (refOperator.m_tagk == TAGK_Proc)
    {
        auto callFrame = PframeNew();

        if (!callFrame)
        {
            //std::printf("RefEvalApply: PframeNew failed\n");
            ret.SetTag(TAGK_Err);
            return ret;
        }

        callFrame->SetSingleParent(proc->m_pframe);

        CPair* formal = proc->m_ppairFormals;
        const int formalCount =
            requiredCount + (hasVarArg != 0);

        for (int i = 0; i < formalCount; ++i)
        {
            if (!formal)
            {
                /*std::printf(
                    "RefEvalApply: missing formal %d of %d\n",
                    i + 1,
                    formalCount);*/

                ret.SetTag(TAGK_Err);
                return ret;
            }

            if (formal->m_ref.m_tagk == TAGK_Pipe)
            {
                formal = formal->m_ppairNext.get();

                if (!formal)
                {
                    /*std::printf(
                        "RefEvalApply: pipe has no vararg formal\n");*/

                    ret.SetTag(TAGK_Err);
                    return ret;
                }
            }

            SYMID symid;

            if (formal->m_ref.m_tagk == TAGK_Symid)
            {
                symid = formal->m_ref.GetSymid();
            }
            else if (formal->m_ref.m_tagk == TAGK_Pair &&
                formal->m_ref.m_ppair)
            {
                CPair* formalName =
                    formal->m_ref.m_ppair.get();

                if (formalName->m_ref.m_tagk != TAGK_Symid)
                {
                    /*std::printf(
                        "RefEvalApply: formal %d does not contain a symbol\n",
                        i + 1);*/

                    ret.SetTag(TAGK_Err);
                    return ret;
                }

                symid = formalName->m_ref.GetSymid();
            }
            else
            {
                /*std::printf(
                    "RefEvalApply: invalid formal tag=%d at index=%d\n",
                    static_cast<int>(formal->m_ref.m_tagk),
                    i);*/

                ret.SetTag(TAGK_Err);
                return ret;
            }

            callFrame->RefAddBinding(symid, &operands[i]);
            formal = formal->m_ppairNext.get();
        }

        ret = RefEvalLambdaBody(proc->m_ppairCodeExpr, callFrame.get());

    }
    else if (refOperator.m_tagk == TAGK_Method)
    {
        if (!method->m_pfnthunk)
        {
#if 0 // Re-enable with the global TAGK_Err audit after the remaining levels are complete.
            BASIC* const pbasic = method->m_pbasic;
            const int cid = pbasic != nullptr && pbasic->pvtbasic != nullptr
                ? static_cast<int>(pbasic->pvtbasic->cid)
                : -1;
            int oid = -1;
            if (pbasic != nullptr && pbasic->pvtbasic != nullptr &&
                FIsBasicDerivedFrom(pbasic, CID_LO))
            {
                oid = static_cast<int>(reinterpret_cast<LO*>(pbasic)->oid);
            }

            const int sourceTag = static_cast<int>(list->m_ref.m_tagk);
            const int symid = list->m_ref.m_tagk == TAGK_Symid
                ? static_cast<int>(list->m_ref.GetSymid())
                : -1;
            const char* symbolName = "-";
            if (g_psw != nullptr && symid >= 0 &&
                static_cast<std::size_t>(symid) < g_psw->mpsymidachz.size())
            {
                symbolName = g_psw->mpsymidachz[symid].c_str();
            }

            std::fprintf(stderr,
                "[TAGK ERROR CONTEXT] reason=null-method-thunk method=%p "
                "basic=%p cid=%d oid=%d required=%d sourceTag=%d "
                "symid=%d name=%s\n",
                static_cast<void*>(method.get()), static_cast<void*>(pbasic),
                cid, oid, requiredCount, sourceTag, symid, symbolName);
            std::fflush(stderr);
#endif

            ret.SetTag(TAGK_Err);
            return ret;
        }

        ret = method->m_pfnthunk(
            method->m_pbasic,
            argumentCount,
            operands);

    }
    else if (refOperator.m_tagk == TAGK_Bifk)
    {
        if (!pbif || !pbif->pfnbif)
        {
            ret.SetTag(TAGK_Err);
            return ret;
        }

        ret = pbif->pfnbif(argumentCount, operands, pframe);

    }

    return ret;
}

CRef RefEvalModule(SW* psw, int isplice)
{
    CRef refRet;

    if (!psw || isplice < 0 || isplice >= psw->csplcSplice)
    {
        refRet.SetTag(TAGK_Void);
        return refRet;
    }

    SPLC& splc = psw->asplcSplice[isplice];

    if (splc.ppairCompile && !splc.pframe)
    {
        splc.pframe = PframeNew();
        refRet = RefEval(splc.ppairCompile.get(), splc.pframe.get());

    }

    refRet.SetTag(TAGK_Void);
    return refRet;
}

BIF g_mpbifkbif[125] = 
{
    { RefOpAdd, 1, 1, "+" },
    { RefOpSub, 1, 1, "-" },
    { RefOpMult, 1, 1, "*" },
    { RefOpDiv, 1, 1, "/" },
    { RefOpPrint, 1, 0, "print" },
    { RefOpPrintFrame, 0, 1, "print-frame" },
    { RefOpPrintSidebag, 1, 0, "print-sidebag" },
    { RefOpIntEqual, 2, 0, "=" },
    { RefOpL, 2, 0, "<" },
    { RefOpLE, 2, 0, "<=" },
    { RefOpG, 2, 0, ">" },
    { RefOpGE, 2, 0, ">=" },
    { RefOpEqv, 2, 0, "eqv?" },
    { RefOpEqual, 2, 0, "equal?" },
    { RefOpIsBoolean, 1, 0, "boolean?" },
    { RefOpIsNum, 1, 0, "number?" },
    { RefOpIsInteger, 1, 0, "integer?" },
    { RefOpIsFloat, 1, 0, "float?" },
    { RefOpIsSymbol, 1, 0, "symbol?" },
    { RefOpIsVector, 1, 0, "vector?" },
    { RefOpIsMatrix, 1, 0, "matrix?" },
    { RefOpIsClq, 1, 0, "clq?" },
    { RefOpIsLm, 1, 0, "lm?" },
    { RefOpIsSmp, 1, 0, "smp?" },
    { RefOpIsList, 1, 0, "list?" },
    { RefOpIsNull, 1, 0, "null?" },
    { RefOpIsObject, 1, 0, "object?" },
    { RefOpIsNullObj, 1, 0, "null-obj?" },
    { RefOpIsMethod, 1, 0, "method?" },
    { RefOpIsProcedure, 1, 0, "procedure?" },
    { RefOpAreNear, 3, 0, "near?" },
    { RefOpNot, 1, 0, "not" },
    { RefOpCons, 2, 0, "cons" },
    { RefOpCar, 1, 0, "car" },
    { RefOpCdr, 1, 0, "cdr" },
    { RefOpSetCar, 2, 0, "set-car!" },
    { RefOpSetCdr, 2, 0, "set-cdr!" },
    { RefOpLength, 1, 0, "length" },
    { RefOpNth, 2, 0, "nth" },
    { RefOpIsMember, 2, 0, "member?" },
    { RefOpList, 0, 1, "list" },
    { RefOpAppend, 0, 1, "append" },
    { RefOpMap, 2, 0, "map" },
    { RefOpFilter, 2, 0, "filter" },
    { RefOpForEach, 2, 0, "for-each" },
    { RefOpEval, 1, 0, "eval" },
    { RefOpVector, 3, 0, "vector" },
    { RefOpMatrix, 0, 1, "matrix" },
    { RefOpClq, 3, 0, "clq" },
    { RefOpLm, 2, 0, "lm" },
    { RefOpSmp, 3, 0, "smp" },
    { RefOpGetElement, 1, 1, "get-element" },
    { RefOpRandomSeed, 1, 0, "random-seed" },
    { RefOpRandom, 2, 0, "random" },
    { RefOpSqrt, 1, 0, "sqrt" },
    { RefOpSin, 1, 0, "sin" },
    { RefOpCos, 1, 0, "cos" },
    { RefOpTan, 1, 0, "tan" },
    { RefOpAsin, 1, 0, "asin" },
    { RefOpAcos, 1, 0, "acos" },
    { RefOpAtan, 1, 0, "atan" },
    { RefOpRadNormalize, 1, 0, "rad-normalize" },
    { RefOpAtan2, 2, 0, "atan2" },
    { RefOpVectorDotProduct, 2, 0, "vector-dot-product" },
    { RefOpVectorCrossProduct, 2, 0, "vector-cross-product" },
    { RefOpVectorLth, 1, 0, "vector-lth" },
    { RefOpVectorDistance, 1, 1, "vector-distance" },
    { RefOpVectorDistanceSquared, 1, 1, "vector-distance-squared" },
    { RefOpVectorNormalize, 1, 1, "vector-normalize" },
    { RefOpVectorProjectNormal, 2, 0, "vector-project-normal" },
    { RefOpVectorProjectTangent, 2, 0, "vector-project-tangent" },
    { RefOpVectorBallisticVelocity, 3, 1, "vector-ballistic-velocity" },
    { RefOpVectorRadianNormal, 2, 0, "vector-radian-normal" },
    { RefOpMatrixTranspose, 1, 0, "matrix-transpose" },
    { RefOpMatrixInvert, 1, 0, "matrix-invert" },
    { RefOpMatrixCalculateDmat, 2, 0, "matrix-calculate-dmat" },
    { RefOpMatrixInterpolateRotate, 3, 0, "matrix-interpolate-rotate" },
    { RefOpMatrixDecomposeToTranslate, 1, 0, "matrix-decompose-to-translate" },
    { RefOpMatrixDecomposeToRotate, 1, 0, "matrix-decompose-to-rotate" },
    { RefOpMatrixDecomposeToEuler, 1, 0, "matrix-decompose-to-euler" },
    { RefOpMatrixDecomposeToRadianNormal, 1, 0, "matrix-decompose-to-radian-normal" },
    { RefOpMatrixLookAt, 2, 1, "matrix-look-at" },
    { RefOpMatrixTiltUpright, 1, 0, "matrix-tilt-upright" },
    { RefOpClqEvaluate, 2, 0, "clq-evaluate" },
    { RefOpClqEvaluateLm, 3, 0, "clq-evaluate-lm" },
    { RefOpClqFit, 4, 0, "clq-fit" },
    { RefOpLmLimit, 2, 0, "lm-limit" },
    { RefOpLmCheck, 2, 0, "lm-check?" },
    { RefOpFloor, 1, 0, "floor" },
    { RefOpCeiling, 1, 0, "ceiling" },
    { RefOpRound, 1, 0, "round" },
    { RefOpTruncate, 1, 0, "truncate" },
    { RefOpAbs, 1, 0, "abs" },
    { RefOpMaximum, 1, 1, "max" },
    { RefOpMinimum, 1, 1, "min" },
    { RefOpModulo, 2, 0, "modulo" },
    { RefOpCurrentTime, 0, 0, "current-time" },
    { RefOpScheduleCallback, 2, 1, "schedule-callback" },
    { RefOpDeferObjectUpdate, 2, 0, "defer-object-update" },
    { RefOpAddO, 3, 0, "addo" },
    { RefOpEnsureO, 3, 0, "ensureo" },
    { RefOpSetO, 3, 0, "seto!" },
    { RefOpGetO, 2, 0, "geto" },
    { RefOpFindObject, 2, 0, "find-object" },
    { RefOpFindObjects, 2, 0, "find-objects" },
    { RefOpFindNearestObject, 2, 0, "find-nearest-object" },
    { RefOpFindNearestObjects, 2, 0, "find-nearest-objects" },
    { RefOpFindPlayerObject, 0, 0, "find-player-object" },
    { RefOpFindWorldObject, 0, 0, "find-world-object" },
    { RefOpFindCameraObject, 0, 0, "find-camera-object" },
    { RefOpFindClassObjects, 2, 0, "find-class-objects" },
    { RefOpFindObjectsInBoundingBox, 2, 0, "find-objects-in-bounding-box" },
    { RefOpFindObjectsInBoundingSphere, 2, 0, "find-objects-in-bounding-sphere" },
    { RefOpHitTestObjects, 3, 0, "hit-test-objects" },
    { RefOpHitTestObjectsFirst, 3, 0, "hit-test-objects-first" },
    { RefOpConvertObjectPosition, 3, 0, "convert-object-position" },
    { RefOpConvertObjectVector, 3, 0, "convert-object-vector" },
    { RefOpConvertObjectMatrix, 3, 0, "convert-object-matrix" },
    { RefOpNearClipCenter, 0, 0, "near-clip-center" },
    { RefOpStartSound, 2, 0, "start-sound" },
    { RefOpStopSound, 1, 0, "stop-sound" },
    { RefOpStartRumble, 2, 0, "start-rumble" },
    { RefOpEmitSmokeCloud, 2, 0, "emit-smoke-cloud" },
    { RefOpPredictAnimationEffect, 7, 0, "predict-animation-effect" },
    { RefOpSetMusicRegister, 2, 0, "set-music-register" },
};
