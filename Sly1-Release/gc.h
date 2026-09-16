#pragma once

class CGc 
{
    public:
    int m_cpframeRoot;
    class CFrame *m_apframeRoot[256];
    int m_cpsidebagRoot;
    class CSidebag *m_apsidebagRoot[128];
    int m_cpframeStack;
    class CFrame *m_apframeStack[512];
    int m_cppairStack;
    class CPair *m_appairStack[256];
    int m_cpprocStack;
    class CProc *m_approcStack[1024];

    void Startup();
    void AddRootFrame(CFrame* pframe);
    void AddRootSidebag(CSidebag* pSidebag);
    void ShutDown();
};

extern CGc g_gc;