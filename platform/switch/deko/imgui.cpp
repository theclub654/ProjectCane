

#include "backend.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <cstddef>
#include <cstdint>

namespace
{
struct Renderer
{
    GLuint program{},font{},vertices{},indices{},array{};
    GLint projection{},texture{};
};
Renderer* Get() { return static_cast<Renderer*>(ImGui::GetIO().BackendRendererUserData); }
void Setup(ImDrawData* data,int width,int height)
{
    auto* r=Get();
    glEnable(GL_BLEND);glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);glDisable(GL_DEPTH_TEST);glDisable(GL_STENCIL_TEST);glEnable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glViewport(0,0,width,height);
    const float left=data->DisplayPos.x,right=left+data->DisplaySize.x;
    const float top=data->DisplayPos.y,bottom=top+data->DisplaySize.y;
    const float matrix[4][4]={{2/(right-left),0,0,0},{0,2/(top-bottom),0,0},{0,0,-1,0},{(right+left)/(left-right),(top+bottom)/(bottom-top),0,1}};
    glUseProgram(r->program);glUniform1i(r->texture,0);glUniformMatrix4fv(r->projection,1,GL_FALSE,&matrix[0][0]);
    glActiveTexture(GL_TEXTURE0);glBindVertexArray(r->array);
    glBindBuffer(GL_ARRAY_BUFFER,r->vertices);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,r->indices);
}
}

bool ImGui_ImplOpenGL3_Init(const char*)
{
    auto& io=ImGui::GetIO();
    IM_ASSERT(!io.BackendRendererUserData);
    io.BackendRendererUserData=new Renderer;
    io.BackendRendererName="ProjectCane_Deko3D";
    io.BackendFlags|=ImGuiBackendFlags_RendererHasVtxOffset;
    return ImGui_ImplOpenGL3_CreateDeviceObjects();
}
void ImGui_ImplOpenGL3_Shutdown()
{
    auto& io=ImGui::GetIO();
    ImGui_ImplOpenGL3_DestroyDeviceObjects();delete Get();
    io.BackendRendererUserData=nullptr;io.BackendRendererName=nullptr;
    io.BackendFlags&=~ImGuiBackendFlags_RendererHasVtxOffset;
}
void ImGui_ImplOpenGL3_NewFrame()
{ if (!Get()->font) ImGui_ImplOpenGL3_CreateFontsTexture(); }
bool ImGui_ImplOpenGL3_CreateFontsTexture()
{
    auto* r=Get();
    unsigned char* pixels;int width,height;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels,&width,&height);
    GLint oldTexture,oldAlignment,oldRowLength;
    glGetIntegerv(GL_TEXTURE_BINDING_2D,&oldTexture);glGetIntegerv(GL_UNPACK_ALIGNMENT,&oldAlignment);glGetIntegerv(GL_UNPACK_ROW_LENGTH,&oldRowLength);
    glGenTextures(1,&r->font);glBindTexture(GL_TEXTURE_2D,r->font);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT,oldAlignment);glPixelStorei(GL_UNPACK_ROW_LENGTH,oldRowLength);glBindTexture(GL_TEXTURE_2D,oldTexture);
    ImGui::GetIO().Fonts->SetTexID(reinterpret_cast<ImTextureID>(uintptr_t(r->font)));
    return true;
}
void ImGui_ImplOpenGL3_DestroyFontsTexture()
{
    auto* r=Get();if (!r || !r->font) return;
    glDeleteTextures(1,&r->font);r->font=0;ImGui::GetIO().Fonts->SetTexID(nullptr);
}
bool ImGui_ImplOpenGL3_CreateDeviceObjects()
{
    auto* r=Get();
    GLint oldArray,oldBuffer;glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldArray);glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&oldBuffer);
    r->program=CaneDeko::CreateProgram("imgui.vert",nullptr,"imgui.frag");
    r->projection=glGetUniformLocation(r->program,"ProjMtx");r->texture=glGetUniformLocation(r->program,"Texture");
    glGenBuffers(1,&r->vertices);glGenBuffers(1,&r->indices);glGenVertexArrays(1,&r->array);
    glBindVertexArray(r->array);glBindBuffer(GL_ARRAY_BUFFER,r->vertices);
    glEnableVertexAttribArray(0);glEnableVertexAttribArray(1);glEnableVertexAttribArray(2);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(ImDrawVert),reinterpret_cast<void*>(offsetof(ImDrawVert,pos)));
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(ImDrawVert),reinterpret_cast<void*>(offsetof(ImDrawVert,uv)));
    glVertexAttribPointer(2,4,GL_UNSIGNED_BYTE,GL_TRUE,sizeof(ImDrawVert),reinterpret_cast<void*>(offsetof(ImDrawVert,col)));
    glBindVertexArray(oldArray);glBindBuffer(GL_ARRAY_BUFFER,oldBuffer);
    return ImGui_ImplOpenGL3_CreateFontsTexture();
}
void ImGui_ImplOpenGL3_DestroyDeviceObjects()
{
    auto* r=Get();if (!r) return;
    ImGui_ImplOpenGL3_DestroyFontsTexture();
    glDeleteBuffers(1,&r->vertices);glDeleteBuffers(1,&r->indices);glDeleteVertexArrays(1,&r->array);glDeleteProgram(r->program);
    r->vertices=r->indices=r->array=r->program=0;
}
void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* data)
{
    const int width=int(data->DisplaySize.x*data->FramebufferScale.x),height=int(data->DisplaySize.y*data->FramebufferScale.y);
    if (width<=0 || height<=0 || !data->TotalVtxCount) return;
    auto* r=Get();
    GLint program,array,buffer,active,texture,viewport[4],scissor[4],colorEquation,alphaEquation,src,dst,asrc,adst;
    GLboolean mask[4];
    glGetIntegerv(GL_CURRENT_PROGRAM,&program);glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&array);glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&buffer);
    glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glActiveTexture(GL_TEXTURE0);glGetIntegerv(GL_TEXTURE_BINDING_2D,&texture);
    glGetIntegerv(GL_VIEWPORT,viewport);glGetIntegerv(GL_SCISSOR_BOX,scissor);glGetBooleanv(GL_COLOR_WRITEMASK,mask);
    glGetIntegerv(GL_BLEND_EQUATION_RGB,&colorEquation);glGetIntegerv(GL_BLEND_EQUATION_ALPHA,&alphaEquation);
    glGetIntegerv(GL_BLEND_SRC_RGB,&src);glGetIntegerv(GL_BLEND_DST_RGB,&dst);glGetIntegerv(GL_BLEND_SRC_ALPHA,&asrc);glGetIntegerv(GL_BLEND_DST_ALPHA,&adst);
    const bool blend=glIsEnabled(GL_BLEND),cull=glIsEnabled(GL_CULL_FACE),depth=glIsEnabled(GL_DEPTH_TEST),stencil=glIsEnabled(GL_STENCIL_TEST),scissorEnabled=glIsEnabled(GL_SCISSOR_TEST);
    Setup(data,width,height);
    for (int listIndex=0;listIndex<data->CmdListsCount;++listIndex)
    {
        const auto* list=data->CmdLists[listIndex];
        glBufferData(GL_ARRAY_BUFFER,list->VtxBuffer.Size*sizeof(ImDrawVert),list->VtxBuffer.Data,GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,list->IdxBuffer.Size*sizeof(ImDrawIdx),list->IdxBuffer.Data,GL_STREAM_DRAW);
        for (const auto& command:list->CmdBuffer)
        {
            if (command.UserCallback)
            {
                if (command.UserCallback==ImDrawCallback_ResetRenderState) Setup(data,width,height);
                else command.UserCallback(list,&command);
                continue;
            }
            const float x0=(command.ClipRect.x-data->DisplayPos.x)*data->FramebufferScale.x;
            const float y0=(command.ClipRect.y-data->DisplayPos.y)*data->FramebufferScale.y;
            const float x1=(command.ClipRect.z-data->DisplayPos.x)*data->FramebufferScale.x;
            const float y1=(command.ClipRect.w-data->DisplayPos.y)*data->FramebufferScale.y;
            if (x1<=x0 || y1<=y0) continue;
            glScissor(int(x0),int(height-y1),int(x1-x0),int(y1-y0));
            glBindTexture(GL_TEXTURE_2D,GLuint(reinterpret_cast<uintptr_t>(command.GetTexID())));
            glDrawElementsBaseVertex(GL_TRIANGLES,command.ElemCount,sizeof(ImDrawIdx)==2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,reinterpret_cast<void*>(uintptr_t(command.IdxOffset*sizeof(ImDrawIdx))),command.VtxOffset);
        }
    }
    glUseProgram(program);glBindVertexArray(array);glBindBuffer(GL_ARRAY_BUFFER,buffer);
    glBindTexture(GL_TEXTURE_2D,texture);glActiveTexture(active);
    glBlendEquationSeparate(colorEquation,alphaEquation);glBlendFuncSeparate(src,dst,asrc,adst);
    (blend?glEnable:glDisable)(GL_BLEND);(cull?glEnable:glDisable)(GL_CULL_FACE);(depth?glEnable:glDisable)(GL_DEPTH_TEST);
    (stencil?glEnable:glDisable)(GL_STENCIL_TEST);(scissorEnabled?glEnable:glDisable)(GL_SCISSOR_TEST);
    glColorMask(mask[0],mask[1],mask[2],mask[3]);glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);glScissor(scissor[0],scissor[1],scissor[2],scissor[3]);
}
