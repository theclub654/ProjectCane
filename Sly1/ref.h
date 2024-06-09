#pragma once

typedef char s8;
typedef short s16;
typedef int s32;
typedef long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;
typedef float f32;
typedef u32 SYMID;

enum TAGK 
{
    TAGK_Nil = -1,
    TAGK_None = 0,
    TAGK_Set = 1,
    TAGK_Define = 2,
    TAGK_DefineMacro = 3,
    TAGK_DefineMacroLambda = 4,
    TAGK_Assert = 5,
    TAGK_If = 6,
    TAGK_Or = 7,
    TAGK_And = 8,
    TAGK_Cond = 9,
    TAGK_Else = 10,
    TAGK_Case = 11,
    TAGK_Let = 12,
    TAGK_While = 13,
    TAGK_Lambda = 14,
    TAGK_Begin = 15,
    TAGK_Import = 16,
    TAGK_Quote = 17,
    TAGK_Dot = 18,
    TAGK_Pipe = 19,
    TAGK_S32 = 20,
    TAGK_F32 = 21,
    TAGK_Vector = 22,
    TAGK_Matrix = 23,
    TAGK_Clq = 24,
    TAGK_Lm = 25,
    TAGK_Smp = 26,
    TAGK_Bool = 27,
    TAGK_Symid = 28,
    TAGK_Bifk = 29,
    TAGK_Pair = 30,
    TAGK_Proc = 31,
    TAGK_Basic = 32,
    TAGK_Method = 33,
    TAGK_Void = 34,
    TAGK_Err = 35,
    TAGK_Max = 36
};
enum BIFK {
    BIFK_Nil = -1,
    BIFK_Add = 0,
    BIFK_Subtract = 1,
    BIFK_Multiply = 2,
    BIFK_Divide = 3,
    BIFK_Print = 4,
    BIFK_PrintFrame = 5,
    BIFK_PrintSidebag = 6,
    BIFK_IntEqual = 7,
    BIFK_Less = 8,
    BIFK_LE = 9,
    BIFK_Greater = 10,
    BIFK_GE = 11,
    BIFK_Eqv = 12,
    BIFK_Equal = 13,
    BIFK_IsBoolean = 14,
    BIFK_IsNumber = 15,
    BIFK_IsInteger = 16,
    BIFK_IsFloat = 17,
    BIFK_IsSymbol = 18,
    BIFK_IsVector = 19,
    BIFK_IsMatrix = 20,
    BIFK_IsClq = 21,
    BIFK_IsLm = 22,
    BIFK_IsSmp = 23,
    BIFK_IsList = 24,
    BIFK_IsNull = 25,
    BIFK_IsObject = 26,
    BIFK_IsNullObj = 27,
    BIFK_IsMethod = 28,
    BIFK_IsProcedure = 29,
    BIFK_AreNear = 30,
    BIFK_Not = 31,
    BIFK_Cons = 32,
    BIFK_Car = 33,
    BIFK_Cdr = 34,
    BIFK_SetCar = 35,
    BIFK_SetCdr = 36,
    BIFK_Length = 37,
    BIFK_Nth = 38,
    BIFK_IsMember = 39,
    BIFK_List = 40,
    BIFK_Append = 41,
    BIFK_Map = 42,
    BIFK_Filter = 43,
    BIFK_ForEach = 44,
    BIFK_Eval = 45,
    BIFK_Vector = 46,
    BIFK_Matrix = 47,
    BIFK_Clq = 48,
    BIFK_Lm = 49,
    BIFK_Smp = 50,
    BIFK_GetElement = 51,
    BIFK_RandomSeed = 52,
    BIFK_Random = 53,
    BIFK_Sqrt = 54,
    BIFK_Sin = 55,
    BIFK_Cos = 56,
    BIFK_Tan = 57,
    BIFK_Asin = 58,
    BIFK_Acos = 59,
    BIFK_Atan = 60,
    BIFK_RadNormalize = 61,
    BIFK_Atan2 = 62,
    BIFK_VectorDotProduct = 63,
    BIFK_VectorCrossProduct = 64,
    BIFK_VectorLth = 65,
    BIFK_VectorDistance = 66,
    BIFK_VectorDistanceSquared = 67,
    BIFK_VectorNormalize = 68,
    BIFK_VectorProjectNormal = 69,
    BIFK_VectorProjectTangent = 70,
    BIFK_VectorBallisticVelocity = 71,
    BIFK_VectorRadianNormal = 72,
    BIFK_MatrixTranspose = 73,
    BIFK_MatrixInvert = 74,
    BIFK_MatrixCalculateDmat = 75,
    BIFK_MatrixInterpolateRotate = 76,
    BIFK_MatrixDecomposeToTranslate = 77,
    BIFK_MatrixDecomposeToRotate = 78,
    BIFK_MatrixDecomposeToEuler = 79,
    BIFK_MatrixDecomposeToRadianNormal = 80,
    BIFK_MatrixLookAt = 81,
    BIFK_MatrixTiltUpright = 82,
    BIFK_ClqEvaluate = 83,
    BIFK_ClqEvaluateLm = 84,
    BIFK_ClqFit = 85,
    BIFK_LmLimit = 86,
    BIFK_LmCheck = 87,
    BIFK_Floor = 88,
    BIFK_Ceiling = 89,
    BIFK_Round = 90,
    BIFK_Truncate = 91,
    BIFK_Abs = 92,
    BIFK_Maximum = 93,
    BIFK_Minimum = 94,
    BIFK_Modulo = 95,
    BIFK_CurrentTime = 96,
    BIFK_ScheduleCallback = 97,
    BIFK_DeferObjectUpdate = 98,
    BIFK_AddO = 99,
    BIFK_EnsureO = 100,
    BIFK_SetO = 101,
    BIFK_GetO = 102,
    BIFK_FindObject = 103,
    BIFK_FindObjects = 104,
    BIFK_FindNearestObject = 105,
    BIFK_FindNearestObjects = 106,
    BIFK_FindPlayerObject = 107,
    BIFK_FindWorldObject = 108,
    BIFK_FindCameraObject = 109,
    BIFK_FindClassObjects = 110,
    BIFK_FindObjectsInBoundingBox = 111,
    BIFK_FindObjectsInBoundingSphere = 112,
    BIFK_HitTestObjects = 113,
    BIFK_HitTestObjectsFirst = 114,
    BIFK_ConvertObjectPosition = 115,
    BIFK_ConvertObjectVector = 116,
    BIFK_ConvertObjectMatrix = 117,
    BIFK_NearClipCenter = 118,
    BIFK_StartSound = 119,
    BIFK_StopSound = 120,
    BIFK_StartRumble = 121,
    BIFK_EmitSmokeCloud = 122,
    BIFK_PredictAnimationEffect = 123,
    BIFK_SetMusicReg = 124,
    BIFK_Max = 125
};

class CREF 
{
    public:
        TAGK m_tagk;

        union 
        {
            s32 m_n;
            f32 m_g;
            int m_bool;
            SYMID m_symid;
            BIFK m_bifk;
            struct CPair* m_ppair;
            struct CProc* m_pproc;
            struct VECTOR* m_pvector;
            struct MATRIX4* m_pmatrix;
            struct CLQ* m_pclq;
            struct LM* m_plm;
            struct SMP* m_psmp;
            struct BASIC* m_pbasic;
            struct CMethod* m_pmethod;
        };
};