#pragma once
#include "alo.h"

enum WMS 
{
    WMS_Nil = -1,
    WMS_Hidden = 0,
    WMS_Appearing = 1,
    WMS_Manual = 2,
    WMS_Warping = 3,
    WMS_Disappearing = 4,
    WMS_Max = 5
};
enum WMD 
{
    WMD_Nil = -1,
    WMD_Up = 0,
    WMD_Down = 1,
    WMD_Left = 2,
    WMD_Right = 3,
    WMD_Max = 4
};
enum WORLDLEVEL 
{
    WORLDLEVEL_Nil = -1,
    WORLDLEVEL_Approach = 0,
    WORLDLEVEL_Hub = 1,
    WORLDLEVEL_1 = 2,
    WORLDLEVEL_2 = 3,
    WORLDLEVEL_3 = 4,
    WORLDLEVEL_4 = 5,
    WORLDLEVEL_5 = 6,
    WORLDLEVEL_6 = 7,
    WORLDLEVEL_Boss = 8,
    WORLDLEVEL_Max = 9
};
struct WMW 
{
    struct ASEG* pasegWarp;
    struct ALO* palo;
    float uAlphaTarget;
    struct PNT* ppnt;
    int fNav;
    int fDot;
    float uDot;
    float uDotTarget;
    float radDot;
    float swDot;
};

class WM : public ALO
{
	public:
        WMS wms;
        float tWms;
        WORLDLEVEL worldlevelCur;
        WORLDLEVEL worldlevelGoal;
        WMS wmsActive;
        struct ASEG* pasegOpen;
        struct ASEG* pasegClose;
        struct ASEGA* pasegaOpenClose;
        struct ASEGA* pasegaMove;
        WMD wmdMoveFrom;
        WORLDLEVEL worldlevelMoveFrom;
        WMW awmw[9];
        WORLDLEVEL worldlevelCursor;
        glm::vec3 posCursor;
        glm::vec3 posCursorTarget;
        int fManualWarp;
};

WM*  NewWm();
int  GetWmSize();
void CloneWm(WM* pwm, WM* pwmBase);
void RenderWmAll(WM* pwm, CM* pcm, RO* pro);
void DeleteWm(WM* pwm);