#include "debug.h"

void RenderMenuGui(SW* psw)
{
    if (ImGui::BeginMainMenuBar()) 
    {
        if (ImGui::BeginMenu("File")) 
        {
            if (ImGui::MenuItem("Open World"))
                instance_a.Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".brx", ".");

            if (ImGui::MenuItem("Close World"))
                DeleteSw(psw);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Render"))
        {
            if (ImGui::MenuItem("Map", "", &fRenderModels))
            {
                
            }

            if (ImGui::MenuItem("Collision", "", &fRenderCollision))
            {
                
            }

            ImGui::EndMenu();
        }

        if (instance_a.Instance()->Display("ChooseFileDlgKey"))
        {
            if (instance_a.Instance()->IsOk() == true)
            {
                file = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                g_transition.m_fPending = 1;
            }

            instance_a.Instance()->Close();
        }

        ImGui::EndMainMenuBar();
    }
}