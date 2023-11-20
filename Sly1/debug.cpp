#include "debug.h"

void RenderOpenFileGui()
{
	ImGui::Begin("ProjectCane");

    // open Dialog Simple
    ImGuiFileDialog instance_a;

    if (ImGui::Button("Open World"))
        instance_a.Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".brx", ".");

    // display
    if (instance_a.Instance()->Display("ChooseFileDlgKey"))
    {
        // action if OK
        if (instance_a.Instance()->IsOk())
        {
            file = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            /*std::cout << file << "\n";
            std::cout << filePath << "\n";*/
            g_transition.m_fPending = 1;
            // action
        }

        // close
        instance_a.Instance()->Close();
    }
}

void RenderCloseWorldGui(SW* psw)
{
    if (ImGui::Button("Close World"))
        DeleteSw(psw);
}
