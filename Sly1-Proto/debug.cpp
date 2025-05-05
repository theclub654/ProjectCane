#include "debug.h"

void RenderMenuGui(SW* psw)
{
    // Start frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    g_fDisableInput = false; // Reset hover flag

    // Main menu bar
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
            g_fDisableInput = true;
        
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open World"))
                instance_a.Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".brx", ".");
            else if (ImGui::IsItemHovered())
                g_fDisableInput = true;
            
            if (ImGui::MenuItem("Close World"))
            {
                if (psw != nullptr)
                {
                    DeleteWorld(psw);
                    file = "";
                    filePath = "";
                    levelName = "";
                }
            }
            else if (ImGui::IsItemHovered())
                g_fDisableInput = true;

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Render"))
        {
            if (ImGui::IsItemHovered()) g_fDisableInput = true;
            if (ImGui::MenuItem("Map", "", &g_fRenderModels));

            if (ImGui::MenuItem("Collision", "", &g_fRenderCollision));
            if (ImGui::IsItemHovered()) g_fDisableInput = true;

            if (ImGui::MenuItem("Cel Borders", "", &g_fRenderCelBorders));
            if (ImGui::IsItemHovered()) g_fDisableInput = true;

            if (ImGui::MenuItem("BSP (Binary Space Partition", "", &g_fBsp));
            if (ImGui::IsItemHovered()) g_fDisableInput = true;

            if (ImGui::BeginMenu("Fog"))
            {
                if (ImGui::MenuItem("Off", nullptr, g_fogType == 0))
                    g_fogType = 0;
                if (ImGui::IsItemHovered()) g_fDisableInput = true;

                if (ImGui::MenuItem("PS2 Style", nullptr, g_fogType == 1))
                    g_fogType = 1;
                if (ImGui::IsItemHovered()) g_fDisableInput = true;

                if (ImGui::MenuItem("PS3 Style", nullptr, g_fogType == 2))
                    g_fogType = 2;
                if (ImGui::IsItemHovered()) g_fDisableInput = true;

                ImGui::EndMenu();

                if (ImGui::IsItemHovered()) g_fDisableInput = true;
            }

            ImGui::SliderFloat("Draw Distance", &g_renderDistance, 1.0f, 2.5);
            
            if (ImGui::IsItemClicked) g_fDisableInput = true;

            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Misc."))
        {
            if (psw != nullptr)
            {
                if (ImGui::MenuItem("Export Map"))
                {
                    std::cout << "Exporting Map..." << "\n";
                    ExportSw();
                    std::cout << "Export Complete" << "\n";
                }
                if (ImGui::IsItemHovered()) g_fDisableInput = true;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // Draw the file dialog
    if (instance_a.Instance()->Display("ChooseFileDlgKey"))
    {
        // Handle file selection
        if (instance_a.Instance()->IsOk())
        {

            if (psw != nullptr)
                DeleteWorld(psw);

            file = ImGuiFileDialog::Instance()->GetFilePathName();
            filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            std::string tempLevelName = ImGuiFileDialog::Instance()->GetCurrentFileName();
            levelName.resize(tempLevelName.size() - 4);

            for (int i = 0; i < tempLevelName.length(); i++)
            {
                char temp = tempLevelName[i];
                if (temp == '.')
                    break;
                else
                    levelName[i] = temp;
            }

            g_transition.m_fPending = 1;
        }

        instance_a.Instance()->Close();
    }
    
    if (ImGuiFileDialog::Instance()->IsOpened("ChooseFileDlgKey"))
        g_fDisableInput = true;
}

void ExportSw()
{
    std::filesystem::create_directory(levelName);
    
    uint32_t modelNum = 0;
    glm::mat4 model{};

    for (int i = 0; i < allSWAloObjs.size(); i++)
    {
        for (int a = 0; a < allSWAloObjs[i]->globset.aglob.size(); a++)
        {
            if (allSWAloObjs[i]->globset.aglob[a].pdmat == nullptr)
            {
                model = allSWAloObjs[i]->xf.matWorld;
                model[3][0] = allSWAloObjs[i]->xf.posWorld.x;
                model[3][1] = allSWAloObjs[i]->xf.posWorld.y;
                model[3][2] = allSWAloObjs[i]->xf.posWorld.z;
            }

            else
            {
                model = allSWAloObjs[i]->xf.matWorld;
                model[3][0] = allSWAloObjs[i]->xf.posWorld.x;
                model[3][1] = allSWAloObjs[i]->xf.posWorld.y;
                model[3][2] = allSWAloObjs[i]->xf.posWorld.z;

                model = model * *allSWAloObjs[i]->globset.aglob[a].pdmat;
            }

            for (int b = 0; b < allSWAloObjs[i]->globset.aglob[a].asubglob.size(); b++)
            {
                std::ofstream objFile(levelName + "/" + "glob" + std::to_string(modelNum) + ".obj");
                modelNum++;

                for (int c = 0; c < allSWAloObjs[i]->globset.aglob[a].asubglob[b].vertices.size(); c++)
                {
                    glm::vec4 vertexHomogeneous = glm::vec4(allSWAloObjs[i]->globset.aglob[a].asubglob[b].vertices[c].pos, 1.0f);
                    glm::vec4 transformedVertexHomogeneous = model * vertexHomogeneous;

                    glm::vec3 vertex = glm::vec3(transformedVertexHomogeneous) / transformedVertexHomogeneous.w;

                    objFile << "v " << std::fixed << std::setprecision(6) << vertex.x << " " << vertex.y << " " << vertex.z << "\n";
                }

                for (int d = 0; d < allSWAloObjs[i]->globset.aglob[a].asubglob[b].vertices.size(); d++)
                {
                    glm::vec3 normal;
                    normal = allSWAloObjs[i]->globset.aglob[a].asubglob[b].vertices[d].normal;

                    objFile << "vn " << normal.x << " " << normal.y << " " << normal.z << "\n";
                }

                for (int e = 0; e < allSWAloObjs[i]->globset.aglob[a].asubglob[b].vertices.size(); e++)
                {
                    glm::vec2 uv;
                    uv = allSWAloObjs[i]->globset.aglob[a].asubglob[b].vertices[e].uv;

                    objFile << "vt " << uv.x << " " << uv.y << "\n";
                }
                
                for (int f = 0; f < allSWAloObjs[i]->globset.aglob[a].asubglob[b].indices.size(); f++)
                {
                    uint16_t indice0 = allSWAloObjs[i]->globset.aglob[a].asubglob[b].indices[f].v1 + 1;
                    uint16_t indice1 = allSWAloObjs[i]->globset.aglob[a].asubglob[b].indices[f].v2 + 1;
                    uint16_t indice2 = allSWAloObjs[i]->globset.aglob[a].asubglob[b].indices[f].v3 + 1;

                    objFile << "f " << indice0 << " " << indice1 << " " << indice2 << "\n";
                }

                objFile.close();
            }
        }
    }
}

bool g_fDisableInput = false;
std::string file = "";
std::string filePath = "";
std::string levelName = "";