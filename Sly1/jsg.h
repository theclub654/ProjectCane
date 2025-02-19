#pragma once
#include "lo.h"

class JSG : public LO
{
	public:
        int cjsge;
        struct JSGE* ajsge;
        struct JT* pjt;
        int ijsgeCur;
        float tCur;
        struct JSGE* pjsgeJoy;
        struct LO* ploContext;
        struct ASEGA* pasegaCur;
        struct LO* ploFocus;
        struct TN* ptnCur;
        int fHideTn;
};

JSG* NewJsg();
void InitJsg(JSG* pjsg);
int  GetJsgSize();
void CloneJsg(JSG* pjsg, JSG* pjsgBase);
void DeleteJsg(JSG* pjsg);