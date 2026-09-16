#pragma once
#include "frame.h"
#include "eval.h"
#include "lo.h"

enum APEK
{
    APEK_Nil = -1,
    APEK_Pos = 1,
    APEK_Rot = 2,
    APEK_Vel = 4,
    APEK_Angvel = 8,
    APEK_Max = 9
};

namespace
{
    bool IsInteger(const CRef& ref);
    bool IsFloat(const CRef& ref);
    bool IsNumber(const CRef& ref);
    CRef MakeNumber(float value, bool forceFloat);
    CRef CompareNumbers(int carg, CRef* aref, bool (*cmp)(float, float));
    bool RefDeepEqual(const CRef& a, const CRef& b);
    static CRef MakeVoidRef();
    static CRef MakeNoneRef();
    static CRef MakeBoolRef(bool value);
    static bool IsFalse(const CRef& ref);
    static bool IsTruthy(const CRef& ref);
    static bool FloatsNear(float a, float b, float epsilon);
    static std::shared_ptr<CPair> CopyList(std::shared_ptr<CPair> head);
    static CRef MakePairRef(std::shared_ptr<CPair> pair);
    static bool IsListLike(const CRef& ref);
    static void AppendNode(std::shared_ptr<CPair>& head, std::shared_ptr<CPair>& tail, const std::shared_ptr<CPair>& node);
    static std::shared_ptr<CPair> MakeQuoteExpr(const CRef& value);
    static CRef EvalCall2(const CRef& callable, const CRef& arg, CFrame* pframe);
    static float RandomFloat01();
    static float RefAsF32(const CRef& ref);
    static int   RefAsS32(const CRef& ref);
    static glm::vec3 RefAsVector(const CRef& ref);
    static glm::mat4 RefAsMatrix(const CRef& ref);
    static bool HasArg(int argc, int index);
    static std::shared_ptr<CPair> MakePairNode(const CRef& ref);
    static int RefAsOptid(const CRef& ref);
    static CLQ RefAsClq(const CRef& ref);
    static LM  RefAsLm(const CRef& ref);
    static BASIC* RefAsBasicRaw(const CRef& ref);
    static LO* RefAsLo(const CRef& ref);
    static ALO* RefAsAlo(const CRef& ref);
    static SO* RefAsSo(const CRef& ref);
    static ASEG* RefAsAseg(const CRef& ref);
    static AMB* RefAsAmb(const CRef& ref);
    static std::shared_ptr<BASIC> BasicSharedFromRaw(BASIC* ptr);
    static CRef RefFromBasic(BASIC* basic);
    static std::shared_ptr<CPair> PairListFromObjects(LO** objects, int count);
    static std::shared_ptr<CPair> PairListFromSoVector(const std::vector<SO*>& objects);
    static CRef RefFromPairList(std::shared_ptr<CPair> list);
}

CRef RefOpAdd(int carg, CRef* aref, CFrame* pframe);
CRef RefOpSub(int carg, CRef* aref, CFrame* pframe);
CRef RefOpMult(int carg, CRef* aref, CFrame* pframe);
CRef RefOpDiv(int carg, CRef* aref, CFrame* pframe);
CRef RefOpPrint(int carg, CRef* aref, CFrame* pframe);
CRef RefOpPrintFrame(int carg, CRef* aref, CFrame* pframe);
CRef RefOpPrintSidebag(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIntEqual(int carg, CRef* aref, CFrame* pframe);
CRef RefOpL(int carg, CRef* aref, CFrame* pframe);
CRef RefOpLE(int carg, CRef* aref, CFrame* pframe);
CRef RefOpG(int carg, CRef* aref, CFrame* pframe);
CRef RefOpGE(int carg, CRef* aref, CFrame* pframe);
CRef RefOpEqv(int carg, CRef* aref, CFrame* pframe);
CRef RefOpEqual(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsBoolean(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsNum(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsInteger(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsFloat(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsSymbol(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsVector(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsMatrix(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsClq(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsLm(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsSmp(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsList(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsNull(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsObject(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsNullObj(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsMethod(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsProcedure(int carg, CRef* aref, CFrame* pframe);
CRef RefOpAreNear(int carg, CRef* aref, CFrame* pframe);
CRef RefOpNot(int carg, CRef* aref, CFrame* pframe);
CRef RefOpCons(int carg, CRef* aref, CFrame* pframe);
CRef RefOpCar(int carg, CRef* aref, CFrame* pframe);
CRef RefOpCdr(int carg, CRef* aref, CFrame* pframe);
CRef RefOpSetCar(int carg, CRef* aref, CFrame* pframe);
CRef RefOpSetCdr(int carg, CRef* aref, CFrame* pframe);
CRef RefOpLength(int carg, CRef* aref, CFrame* pframe);
CRef RefOpNth(int carg, CRef* aref, CFrame* pframe);
CRef RefOpIsMember(int carg, CRef* aref, CFrame* pframe);
CRef RefOpList(int carg, CRef* aref, CFrame* pframe);
CRef RefOpAppend(int carg, CRef* aref, CFrame* pframe);
CRef RefOpMap(int carg, CRef* aref, CFrame* pframe);
CRef RefOpFilter(int carg, CRef* aref, CFrame* pframe);
CRef RefOpForEach(int carg, CRef* aref, CFrame* pframe);
CRef RefOpEval(int carg, CRef* aref, CFrame* pframe);
CRef RefOpVector(int carg, CRef* aref, CFrame* pframe);
CRef RefOpMatrix(int carg, CRef* aref, CFrame* pframe);
CRef RefOpClq(int carg, CRef* aref, CFrame* pframe);
CRef RefOpLm(int carg, CRef* aref, CFrame* pframe);
CRef RefOpSmp(int carg, CRef* aref, CFrame* pframe);
CRef RefOpGetElement(int carg, CRef* aref, CFrame* pframe);
CRef RefOpRandomSeed(int carg, CRef* aref, CFrame* pframe);
CRef RefOpRandom(int carg, CRef* aref, CFrame* pframe);
CRef RefOpSqrt(int carg, CRef* aref, CFrame* pframe);
CRef RefOpSin(int carg, CRef* aref, CFrame* pframe);
CRef RefOpCos(int carg, CRef* aref, CFrame* pframe);
CRef RefOpTan(int carg, CRef* aref, CFrame* pframe);
CRef RefOpAsin(int carg, CRef* aref, CFrame* pframe);
CRef RefOpAcos(int carg, CRef* aref, CFrame* pframe);
CRef RefOpAtan(int argc, CRef* argv, CFrame* pframe);
CRef RefOpRadNormalize(int argc, CRef* argv, CFrame* pframe);
CRef RefOpAtan2(int argc, CRef* argv, CFrame* pframe);
CRef RefOpVectorDotProduct(int argc, CRef* argv, CFrame* pframe);
CRef RefOpVectorCrossProduct(int argc, CRef* argv, CFrame* pframe);
CRef RefOpVectorLth(int argc, CRef* argv, CFrame* pframe);
CRef RefOpVectorDistance(int argc, CRef* argv, CFrame* pframe);
CRef RefOpVectorDistanceSquared(int argc, CRef* argv, CFrame* pframe);
CRef RefOpVectorNormalize(int argc, CRef* argv, CFrame* pframe);
CRef RefOpVectorProjectNormal(int argc, CRef* argv, CFrame* pframe);
CRef RefOpVectorProjectTangent(int argc, CRef* argv, CFrame* pframe);
CRef RefOpVectorBallisticVelocity(int argc, CRef* argv, CFrame* pframe);
CRef RefOpVectorRadianNormal(int argc, CRef* argv, CFrame* pframe);
CRef RefOpMatrixTranspose(int argc, CRef* argv, CFrame* pframe);
CRef RefOpMatrixInvert(int argc, CRef* argv, CFrame* pframe);
CRef RefOpMatrixCalculateDmat(int argc, CRef* argv, CFrame* pframe);
CRef RefOpMatrixInterpolateRotate(int argc, CRef* argv, CFrame* pframe);
CRef RefOpMatrixDecomposeToTranslate(int argc, CRef* argv, CFrame* pframe);
CRef RefOpMatrixDecomposeToRotate(int argc, CRef* argv, CFrame* pframe);
CRef RefOpMatrixDecomposeToEuler(int argc, CRef* argv, CFrame* pframe);
CRef RefOpMatrixDecomposeToRadianNormal(int argc, CRef* argv, CFrame* pframe);
CRef RefOpMatrixLookAt(int argc, CRef* argv, CFrame* pframe);
CRef RefOpMatrixTiltUpright(int argc, CRef* argv, CFrame* pframe);
CRef RefOpClqEvaluate(int argc, CRef* argv, CFrame* pframe);
CRef RefOpClqEvaluateLm(int argc, CRef* argv, CFrame* pframe);
CRef RefOpClqFit(int argc, CRef* argv, CFrame* pframe);
CRef RefOpLmLimit(int argc, CRef* argv, CFrame* pframe);
CRef RefOpLmCheck(int argc, CRef* argv, CFrame* pframe);
CRef RefOpFloor(int argc, CRef* argv, CFrame* pframe);
CRef RefOpCeiling(int argc, CRef* argv, CFrame* pframe);
CRef RefOpRound(int argc, CRef* argv, CFrame* pframe);
CRef RefOpTruncate(int argc, CRef* argv, CFrame* pframe);
CRef RefOpAbs(int argc, CRef* argv, CFrame* pframe);
CRef RefOpMaximum(int argc, CRef* argv, CFrame* pframe);
CRef RefOpMinimum(int argc, CRef* argv, CFrame* pframe);
CRef RefOpModulo(int argc, CRef* argv, CFrame* pframe);
CRef RefOpCurrentTime(int argc, CRef* argv, CFrame* pframe);
CRef RefOpScheduleCallback(int argc, CRef* argv, CFrame* pframe);
CRef RefOpDeferObjectUpdate(int argc, CRef* argv, CFrame* pframe);
CRef RefOpAddO(int argc, CRef* argv, CFrame* pframe);
CRef RefOpObjectOption(int argc, CRef* argv, CFrame* pframe, BIFK bifk);
CRef RefOpEnsureO(int argc, CRef* argv, CFrame* pframe);
CRef RefOpSetO(int argc, CRef* argv, CFrame* pframe);
CRef RefOpGetO(int argc, CRef* argv, CFrame* pframe);
CRef RefOpFindObject(int argc, CRef* argv, CFrame* pframe);
CRef RefOpFindObjects(int argc, CRef* argv, CFrame* pframe);
CRef RefOpFindNearestObject(int argc, CRef* argv, CFrame* pframe);
CRef RefOpFindNearestObjects(int argc, CRef* argv, CFrame* pframe);
CRef RefOpFindPlayerObject(int argc, CRef* argv, CFrame* pframe);
CRef RefOpFindWorldObject(int argc, CRef* argv, CFrame* pframe);
CRef RefOpFindCameraObject(int argc, CRef* argv, CFrame* pframe);
CRef RefOpFindClassObjects(int argc, CRef* argv, CFrame* pframe);
CRef RefOpFindObjectsInBoundingBox(int argc, CRef* argv, CFrame* pframe);
CRef RefOpFindObjectsInBoundingSphere(int argc, CRef* argv, CFrame* pframe);
CRef RefOpHitTestObjectsImpl(BIFK bifk, int carg, CRef* aref, CFrame* pframe);
CRef RefOpHitTestObjects(int argc, CRef* argv, CFrame* pframe);
CRef RefOpHitTestObjectsFirst(int argc, CRef* argv, CFrame* pframe);
CRef RefOpConvertObjectPosition(int argc, CRef* argv, CFrame* pframe);
CRef RefOpConvertObjectVector(int argc, CRef* argv, CFrame* pframe);
CRef RefOpConvertObjectMatrix(int argc, CRef* argv, CFrame* pframe);
CRef RefOpNearClipCenter(int argc, CRef* argv, CFrame* pframe);
CRef RefOpStartSound(int argc, CRef* argv, CFrame* pframe);
CRef RefOpStopSound(int argc, CRef* argv, CFrame* pframe);
CRef RefOpStartRumble(int argc, CRef* argv, CFrame* pframe);
CRef RefOpEmitSmokeCloud(int argc, CRef* argv, CFrame* pframe);
CRef RefOpPredictAnimationEffect(int argc, CRef* argv, CFrame* pframe);
CRef RefOpSetMusicRegister(int argc, CRef* argv, CFrame* pframe);
