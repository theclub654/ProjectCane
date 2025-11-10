#include "debug.h"
#define  STB_IMAGE_WRITE_IMPLEMENTATION 
#include <stb/stb_image_write.h>

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
                {
                    glGlobShader.Use();
                    g_fogType = 0;
                    glUniform1i(glslFogType, g_fogType);
                }
               
                if (ImGui::IsItemHovered()) g_fDisableInput = true;

                if (ImGui::MenuItem("PS2 Style", nullptr, g_fogType == 1))
                {
                    glGlobShader.Use();
                    g_fogType = 1; 
                    glUniform1i(glslFogType, g_fogType);
                }
                   
                if (ImGui::IsItemHovered()) g_fDisableInput = true;

                if (ImGui::MenuItem("PS3 Style", nullptr, g_fogType == 2))
                {
                    glGlobShader.Use();
                    g_fogType = 2; 
                    glUniform1i(glslFogType, g_fogType);
                }

                if (ImGui::IsItemHovered()) g_fDisableInput = true;

                ImGui::EndMenu();

                if (ImGui::IsItemHovered()) g_fDisableInput = true;
            }

            if (ImGui::BeginMenu("Aspect Ratio"))
            {
                if (ImGui::MenuItem("Fit to screen", nullptr, g_gl.aspectMode == FitToScreen)) {
                    g_gl.aspectMode = FitToScreen;
                    g_gl.aspectRatio = g_gl.width / g_gl.height;
                    int width, height;
                    glfwGetFramebufferSize(g_gl.window, &width, &height);
                    FrameBufferSizeCallBack(g_gl.window, width, height);
                }

                if (ImGui::MenuItem("4:3", nullptr, g_gl.aspectMode == Fixed_4_3)) {
                    g_gl.aspectMode = Fixed_4_3;
                    g_gl.aspectRatio = 4.0f / 3.0f;
                    int width, height;
                    glfwGetFramebufferSize(g_gl.window, &width, &height);
                    FrameBufferSizeCallBack(g_gl.window, width, height);
                }

                if (ImGui::MenuItem("16:9", nullptr, g_gl.aspectMode == Fixed_16_9)) {
                    g_gl.aspectMode = Fixed_16_9;
                    g_gl.aspectRatio = 16.0f / 9.0f;
                    int width, height;
                    glfwGetFramebufferSize(g_gl.window, &width, &height);
                    FrameBufferSizeCallBack(g_gl.window, width, height);
                }

                if (ImGui::MenuItem("16:10", nullptr, g_gl.aspectMode == Fixed_16_10)) {
                    g_gl.aspectMode = Fixed_16_10;
                    g_gl.aspectRatio = 16.0f / 10.0f;
                    int width, height;
                    glfwGetFramebufferSize(g_gl.window, &width, &height);
                    FrameBufferSizeCallBack(g_gl.window, width, height);
                }

                ImGui::EndMenu();
            }
           
            if (g_pcm != nullptr)
                ImGui::SliderFloat("Draw Distance", &g_pcm->rMRDAdjust, 1.0f, 3.0f);

            if (ImGui::IsItemClicked) g_fDisableInput = true;

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Misc."))
        {
            if (psw != nullptr)
            {
                if (ImGui::MenuItem("Export Textures"))
                {
                    std::cout << "Exporting Textures..." << "\n";
                    ExportTextures();
                    std::cout << "Export Complete" << "\n";
                }
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
                int shdID = allSWAloObjs[i]->globset.aglob[a].asubglob[b].shdID;
                std::string baseName = "glob" + std::to_string(modelNum);
                std::string objPath = levelName + "/" + baseName + ".obj";
                std::string mtlPath = baseName + ".mtl";

                // Padded texture name
                char texName[64];
                snprintf(texName, sizeof(texName), "shader_%03d_diffuse.png", shdID);

                std::ofstream objFile(objPath);
                std::ofstream mtlFile(levelName + "/" + mtlPath);

                // Write MTL
                mtlFile << "newmtl material_0\n";
                mtlFile << "Ka 1.000 1.000 1.000\n";
                mtlFile << "Kd 1.000 1.000 1.000\n";
                mtlFile << "Ks 0.000 0.000 0.000\n";
                mtlFile << "d 1.0\n";
                mtlFile << "illum 2\n";
                mtlFile << "map_Kd Textures/" << texName << "\n";
                mtlFile.close();

                // Write OBJ
                objFile << "mtllib " << mtlPath << "\n";
                objFile << "usemtl material_0\n";

                auto& sub = allSWAloObjs[i]->globset.aglob[a].asubglob[b];

                for (auto& v : sub.vertices) {
                    glm::vec4 vt = model * glm::vec4(v.pos, 1.0f);
                    glm::vec3 vertex = glm::vec3(vt) / vt.w;
                    objFile << "v " << std::fixed << std::setprecision(6)
                        << vertex.x << " " << vertex.y << " " << vertex.z << "\n";
                }

                for (auto& v : sub.vertices)
                    objFile << "vn " << v.normal.x << " " << v.normal.y << " " << v.normal.z << "\n";

                for (auto& v : sub.vertices)
                    objFile << "vt " << v.uv.x << " " << v.uv.y << "\n";

                for (auto& tri : sub.indices)
                    objFile << "f "
                    << (tri.v1 + 1) << "/" << (tri.v1 + 1) << "/" << (tri.v1 + 1) << " "
                    << (tri.v2 + 1) << "/" << (tri.v2 + 1) << "/" << (tri.v2 + 1) << " "
                    << (tri.v3 + 1) << "/" << (tri.v3 + 1) << "/" << (tri.v3 + 1) << "\n";

                objFile.close();
                modelNum++;
            }
        }
    }

    ExportTextures();
}

void ExportTextures()
{
    std::string textureDir = levelName + "/Textures";
    std::filesystem::create_directories(textureDir);

    for (size_t i = 0; i < g_ashd.size(); ++i)
    {
        SHD& shd = g_ashd[i];
        BMP* bmp = shd.atex[0].abmp[0];

        if (!bmp || bmp->diffuseTexture.empty())
            continue;

        char filename[256];
        snprintf(filename, sizeof(filename), "%s/shader_%03zu_diffuse.png", textureDir.c_str(), i);

        stbi_flip_vertically_on_write(1);
        stbi_write_png(filename, bmp->bmpWidth, bmp->bmpHeight, 4, bmp->diffuseTexture.data(), bmp->bmpWidth * 4);
        stbi_flip_vertically_on_write(0);
    }

    for (size_t i = 0; i < g_afontBrx.size(); ++i)
    {
        BMP* bmp = g_afontBrx[i].m_pbmp;

        if (!bmp || bmp->diffuseTexture.empty())
            continue;

        char filename[256];
        snprintf(filename, sizeof(filename), "%s/font_%03zu.png", textureDir.c_str(), i);

        stbi_write_png(filename, bmp->bmpWidth, bmp->bmpHeight, 4, bmp->diffuseTexture.data(), bmp->bmpWidth * 4);
    }
}

bool g_fDisableInput = false;
std::string file = "";
std::string filePath = "";
std::string levelName = "";