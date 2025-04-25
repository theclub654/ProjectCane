#include "path.h"

PATHZONE* NewPathzone()
{
    return new PATHZONE{};
}

void InitSwPathzoneDl(SW* psw)
{
    InitDl(&psw->dlPathzone, offsetof(PATHZONE, dlePathzone));
}

int GetPathzoneSize()
{
    return sizeof(PATHZONE);
}

void OnPathzoneAdd(PATHZONE* ppathzone)
{
    OnLoAdd(ppathzone);
    AppendDlEntry(&ppathzone->psw->dlPathzone, ppathzone);
}

void OnPathzoneRemove(PATHZONE* ppathzone)
{
    OnLoRemove(ppathzone);
    RemoveDlEntry(&ppathzone->psw->dlPathzone, ppathzone);
}

void LoadPathZoneFromBrx(PATHZONE* ppathzone, CBinaryInputStream* pbis)
{
    LoadLoFromBrx(ppathzone, pbis);

    ppathzone->cg.ccgv = pbis->U16Read();
    ppathzone->cg.acgv.resize(ppathzone->cg.ccgv);

    ppathzone->cg.ccgeBoundary = pbis->U16Read();
    ppathzone->cg.ccge = ppathzone->cg.ccgeBoundary + pbis->U16Read();
    ppathzone->cg.acge.resize(ppathzone->cg.ccge);

    ppathzone->cg.ccgt = pbis->U16Read();
    ppathzone->cg.acgt.resize(ppathzone->cg.ccgt);

    for (int i = 0; i < ppathzone->cg.ccgv; i++)
    {
        ppathzone->cg.acgv[i].pos = pbis->ReadVector();

        byte unk_4 = pbis->U8Read();

        for (int i = 0; i < unk_4; i++)
            pbis->U16Read();
    }

    for (int i = 0; i < ppathzone->cg.ccge; i++)
    {
        for (int i = 0; i < 2; i++)
            pbis->U16Read();
    }

    for (int i = 0; i < ppathzone->cg.ccgt; i++)
    {
        for (int i = 0; i <= 2; i++)
            pbis->U16Read();
    }

    uint16_t unk_5 = pbis->U16Read();
    for (int i = 0; i < unk_5; i++)
    {
        pbis->ReadVector();
        pbis->F32Read();
        pbis->S16Read();
        pbis->S16Read();
    }
}

void ClonePathzone(PATHZONE* ppathzone, PATHZONE* ppathzoneBase)
{
    CloneLo(ppathzone, ppathzoneBase);

    // Shallow copy fields
    ppathzone->VAO = ppathzoneBase->VAO;
    ppathzone->VBO = ppathzoneBase->VBO;
    ppathzone->EBO = ppathzoneBase->EBO;
    ppathzone->cvtx = ppathzoneBase->cvtx;

    ppathzone->cg = ppathzoneBase->cg;
    ppathzone->dlePathzone = ppathzoneBase->dlePathzone;
}

void RenderPathzone(PATHZONE* ppathzone, CM* pcm)
{
    RPL rpl{};

    rpl.PFNDRAW = DrawPathzone;

    rpl.ro.modelmatrix = glm::identity <glm::mat4>();

    rpl.ro.VAO = &ppathzone->VAO;
    rpl.rp = RP_Opaque;
    rpl.ro.cvtx = ppathzone->cvtx;

    SubmitRpl(&rpl);
}

void DrawPathzone(RPL* prpl)
{
    glUniform1i(glGetUniformLocation(glGlobShader.ID, "rko"), 4);
    glUniformMatrix4fv(glGetUniformLocation(glGlobShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(prpl->ro.modelmatrix));

    glBindVertexArray(*prpl->ro.VAO);
    //glDrawArrays(GL_LINE_STRIP, 0, prpl->ro.cvtx);
    glDrawElements(GL_LINES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);
}

void DeletePathzone(PATHZONE* ppathzone)
{
    glDeleteVertexArrays(1, &ppathzone->VAO);
    glDeleteBuffers(1, &ppathzone->VBO);
    glDeleteBuffers(1, &ppathzone->EBO);

    delete ppathzone;
}
