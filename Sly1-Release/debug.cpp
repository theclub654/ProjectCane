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

            if (g_pcm != nullptr)
                ImGui::SliderFloat("Draw Distance", &g_pcm->rMRDAdjust, baseRenderDistance, 4.0f);

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

                if (ImGui::MenuItem("Export Map (Bake Lights)"))
                {
                    std::cout << "Exporting Map..." << "\n";
                    ExportSwLightsBaked();
                    std::cout << "Export Complete" << "\n";
                }

                if (ImGui::MenuItem("Export Textures"))
                {
                    std::cout << "Exporting Textures..." << "\n";
                    ExportTextures();
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

void AppendFloat(std::vector<uint8_t>& buf, float v)
{
    uint8_t bytes[4];
    std::memcpy(bytes, &v, 4);
    buf.insert(buf.end(), bytes, bytes + 4);
}

void AppendUInt32(std::vector<uint8_t>& buf, uint32_t v)
{
    uint8_t bytes[4];
    std::memcpy(bytes, &v, 4);
    buf.insert(buf.end(), bytes, bytes + 4);
}

void AlignTo4(std::vector<uint8_t>& buf)
{
    while (buf.size() % 4 != 0)
        buf.push_back(0);
}

void PngWriteCallback(void* context, void* data, int size)
{
    PngBuffer* png = reinterpret_cast<PngBuffer*>(context);
    uint8_t* bytes = reinterpret_cast<uint8_t*>(data);
    png->data.insert(png->data.end(), bytes, bytes + size);
}

glm::quat RotationBetweenVectors(const glm::vec3& from, const glm::vec3& to)
{
    glm::vec3 f = glm::normalize(from);
    glm::vec3 t = glm::normalize(to);

    float cosTheta = glm::dot(f, t);

    if (cosTheta > 0.9999f)
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    if (cosTheta < -0.9999f)
    {
        glm::vec3 axis = glm::cross(glm::vec3(0, 1, 0), f);
        if (glm::length2(axis) < 1e-6f)
            axis = glm::cross(glm::vec3(1, 0, 0), f);

        axis = glm::normalize(axis);
        return glm::angleAxis(glm::pi<float>(), axis);
    }

    glm::vec3 axis = glm::normalize(glm::cross(f, t));
    float s = glm::sqrt((1.0f + cosTheta) * 2.0f);
    float invS = 1.0f / s;

    glm::quat q;
    q.w = s * 0.5f;
    q.x = axis.x * invS;
    q.y = axis.y * invS;
    q.z = axis.z * invS;
    return glm::normalize(q);
}

void ExportSw()
{
    std::filesystem::create_directory(levelName);

    // One buffer for everything (geo + textures)
    std::vector<uint8_t> binaryBuffer;

    // glTF JSON pieces
    json gltf;
    std::vector<json> bufferViews;
    std::vector<json> accessors;
    std::vector<json> materials;
    std::vector<json> images;
    std::vector<json> textures;
    std::vector<json> meshes;
    std::vector<json> nodes;
    std::vector<int>  sceneNodes;

    // Asset header
    gltf["asset"] = {
        {"version", "2.0"},
        {"generator", "ExportSwGLB"}
    };

    // Default material (no texture)
    int defaultMaterialIndex = 0;
    materials.push_back({
        {"name", "DefaultMaterial"},
        {"pbrMetallicRoughness", {
            {"baseColorFactor", {1.0, 1.0, 1.0, 1.0}},
            {"metallicFactor", 0.0},
            {"roughnessFactor", 1.0}
        }}
        });

    // Cache: shdID material index (so we reuse images/textures)
    std::unordered_map<int, int> shdToMaterialIndex;

    uint32_t modelNum = 0;
    glm::mat4 model(1.0f);

    // change to y up
    glm::mat4 zUpToYUp = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));

    // ===================== GEOMETRY & TEXTURES -> binaryBuffer =====================
    for (int i = 0; i < (int)allSWAloObjs.size(); ++i)
    {
        auto* alo = allSWAloObjs[i];
        if (!alo) continue;

        // One GLOBSET per ALO
        int globsetNodeIndex = (int)nodes.size();
        {
            json globsetNode = {
                {"name", "GLOBSET_" + std::to_string(i)},
                {"children", json::array()}
            };
            nodes.push_back(globsetNode);
            sceneNodes.push_back(globsetNodeIndex);
        }

        for (int a = 0; a < (int)alo->globset.aglob.size(); ++a)
        {
            auto& glob = alo->globset.aglob[a];

            // Game world matrix (same as OBJ)
            if (glob.pdmat == nullptr)
            {
                model = alo->xf.matWorld;
                model[3][0] = alo->xf.posWorld.x;
                model[3][1] = alo->xf.posWorld.y;
                model[3][2] = alo->xf.posWorld.z;
            }
            else
            {
                model = alo->xf.matWorld;
                model[3][0] = alo->xf.posWorld.x;
                model[3][1] = alo->xf.posWorld.y;
                model[3][2] = alo->xf.posWorld.z;
                model = model * *glob.pdmat;
            } 

            glm::mat4 finalModel = zUpToYUp * model;
            glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(finalModel)));

            // GLOB node under this ALO's GLOBSET
            int globNodeIndex = (int)nodes.size();
            {
                json globNode = {
                    {"name", "GLOB_" + std::to_string(i) + "_" + std::to_string(a)},
                    {"children", json::array()}
                };
                nodes.push_back(globNode);
                nodes[globsetNodeIndex]["children"].push_back(globNodeIndex);
            }

            for (int b = 0; b < (int)glob.asubglob.size(); ++b)
            {
                auto& sub = glob.asubglob[b];
                const size_t vertCount = sub.vertices.size();
                const size_t triCount = sub.indices.size();

                if (vertCount == 0 || triCount == 0)
                {
                    ++modelNum;
                    continue;
                }

                std::string baseName =
                    "SUBGLOB_" + std::to_string(i) + "_" +
                    std::to_string(a) + "_" +
                    std::to_string(b);

                // ========= MATERIAL / EMBEDDED TEXTURE for this subglob =========
                int materialIndex = defaultMaterialIndex;

                int shdID = sub.shdID;
                if (shdID >= 0 && shdID < (int)g_ashd.size())
                {
                    auto it = shdToMaterialIndex.find(shdID);
                    if (it == shdToMaterialIndex.end())
                    {
                        bool wantsAlphaBlend =
                            glob.rp == RP_Cutout ||
                            glob.rp == RP_CutoutAfterProjVolume ||
                            glob.rp == RP_Translucent ||
                            (glob.rp == RP_Background && shdID >= 0 && shdID < (int)g_ashd.size() && g_ashd[shdID].grfshd == 2);

                        SHD& shd = g_ashd[shdID];
                        BMP* bmp = shd.atex[0].abmp[0];

                        if (bmp && !bmp->diffuseTexture.empty())
                        {
                            // ---- Encode BMP as PNG into memory ----
                            PngBuffer png;

                            stbi_write_png_to_func(
                                PngWriteCallback,
                                &png,
                                bmp->bmpWidth,
                                bmp->bmpHeight,
                                4,
                                bmp->diffuseTexture.data(),
                                bmp->bmpWidth * 4
                            );

                            // ---- Append PNG bytes to binaryBuffer ----
                            AlignTo4(binaryBuffer);
                            size_t pngByteOffset = binaryBuffer.size();
                            binaryBuffer.insert(binaryBuffer.end(),
                                png.data.begin(), png.data.end());
                            size_t pngByteLength = png.data.size();

                            int pngBVIndex = (int)bufferViews.size();
                            bufferViews.push_back({
                                {"buffer", 0},
                                {"byteOffset", pngByteOffset},
                                {"byteLength", pngByteLength}
                                });

                            int imageIndex = (int)images.size();
                            images.push_back({
                                {"bufferView", pngBVIndex},
                                {"mimeType", "image/png"}
                                });

                            int textureIndex = (int)textures.size();
                            textures.push_back({
                                {"source", imageIndex}
                                });

                            json mat = {
                                {"name", "Material_Shader_" + std::to_string(shdID) +
                                         (wantsAlphaBlend ? "_Alpha" : "_Opaque")},
                                {"pbrMetallicRoughness", {
                                    {"baseColorTexture", { {"index", textureIndex} }},
                                    {"metallicFactor", 0.0},
                                    {"roughnessFactor", 1.0}
                                }}
                            };

                            // Alpha mode selection
                            if (wantsAlphaBlend)
                            {
                                mat["alphaMode"] = "BLEND";
                                mat["alphaCutoff"] = 0.0;
                                mat["doubleSided"] = true;
                            }
                            else
                            {
                                mat["alphaMode"] = "OPAQUE";
                            }

                            materialIndex = (int)materials.size();
                            materials.push_back(mat);
                            shdToMaterialIndex[shdID] = materialIndex;
                        }
                        else
                        {
                            materialIndex = defaultMaterialIndex;
                            shdToMaterialIndex[shdID] = materialIndex;
                        }
                    }
                    else
                    {
                        materialIndex = it->second;
                    }
                }
                else
                {
                    materialIndex = defaultMaterialIndex;
                }

                // ========= POSITIONS =========
                AlignTo4(binaryBuffer);
                size_t posByteOffset = binaryBuffer.size();

                for (auto& v : sub.vertices)
                {
                    glm::vec4 vt = finalModel * glm::vec4(v.pos, 1.0f);
                    glm::vec3 p = glm::vec3(vt) / vt.w;

                    AppendFloat(binaryBuffer, p.x);
                    AppendFloat(binaryBuffer, p.y);
                    AppendFloat(binaryBuffer, p.z);
                }
                size_t posByteLength = binaryBuffer.size() - posByteOffset;

                int posBVIndex = (int)bufferViews.size();
                bufferViews.push_back({
                    {"buffer", 0},
                    {"byteOffset", posByteOffset},
                    {"byteLength", posByteLength},
                    {"target", 34962} // ARRAY_BUFFER
                    });

                int posAccessorIndex = (int)accessors.size();
                accessors.push_back({
                    {"bufferView", posBVIndex},
                    {"byteOffset", 0},
                    {"componentType", 5126}, // FLOAT
                    {"count", vertCount},
                    {"type", "VEC3"}
                    });

                // ========= NORMALS =========
                AlignTo4(binaryBuffer);
                size_t nrmByteOffset = binaryBuffer.size();

                for (auto& v : sub.vertices)
                {
                    AppendFloat(binaryBuffer, v.normal.x);
                    AppendFloat(binaryBuffer, v.normal.y);
                    AppendFloat(binaryBuffer, v.normal.z);
                }
                size_t nrmByteLength = binaryBuffer.size() - nrmByteOffset;

                int nrmBVIndex = (int)bufferViews.size();
                bufferViews.push_back({
                    {"buffer", 0},
                    {"byteOffset", nrmByteOffset},
                    {"byteLength", nrmByteLength},
                    {"target", 34962}
                    });

                int nrmAccessorIndex = (int)accessors.size();
                accessors.push_back({
                    {"bufferView", nrmBVIndex},
                    {"byteOffset", 0},
                    {"componentType", 5126}, // FLOAT
                    {"count", vertCount},
                    {"type", "VEC3"}
                    });

                // ========= COLORS (COLOR_0) =========
                AlignTo4(binaryBuffer);
                size_t colByteOffset = binaryBuffer.size();

                for (auto& v : sub.vertices)
                {
                    AppendFloat(binaryBuffer, v.color.r);
                    AppendFloat(binaryBuffer, v.color.g);
                    AppendFloat(binaryBuffer, v.color.b);
                    AppendFloat(binaryBuffer, v.color.a);
                }
                size_t colByteLength = binaryBuffer.size() - colByteOffset;

                int colBVIndex = (int)bufferViews.size();
                bufferViews.push_back({
                    {"buffer", 0},
                    {"byteOffset", colByteOffset},
                    {"byteLength", colByteLength},
                    {"target", 34962}
                    });

                int colAccessorIndex = (int)accessors.size();
                accessors.push_back({
                    {"bufferView", colBVIndex},
                    {"byteOffset", 0},
                    {"componentType", 5126}, // FLOAT
                    {"count", vertCount},
                    {"type", "VEC4"}
                    });

                // ========= UVs (TEXCOORD_0) =========
                AlignTo4(binaryBuffer);
                size_t uvByteOffset = binaryBuffer.size();

                for (auto& v : sub.vertices)
                {
                    AppendFloat(binaryBuffer, v.uv.x);
                    AppendFloat(binaryBuffer, v.uv.y);
                }
                size_t uvByteLength = binaryBuffer.size() - uvByteOffset;

                int uvBVIndex = (int)bufferViews.size();
                bufferViews.push_back({
                    {"buffer", 0},
                    {"byteOffset", uvByteOffset},
                    {"byteLength", uvByteLength},
                    {"target", 34962}
                    });

                int uvAccessorIndex = (int)accessors.size();
                accessors.push_back({
                    {"bufferView", uvBVIndex},
                    {"byteOffset", 0},
                    {"componentType", 5126}, // FLOAT
                    {"count", vertCount},
                    {"type", "VEC2"}
                    });

                // ========= INDICES =========
                AlignTo4(binaryBuffer);
                size_t idxByteOffset = binaryBuffer.size();
                uint32_t indexCount = (uint32_t)(triCount * 3);

                for (auto& tri : sub.indices)
                {
                    AppendUInt32(binaryBuffer, (uint32_t)tri.v1);
                    AppendUInt32(binaryBuffer, (uint32_t)tri.v2);
                    AppendUInt32(binaryBuffer, (uint32_t)tri.v3);
                }
                size_t idxByteLength = binaryBuffer.size() - idxByteOffset;

                int idxBVIndex = (int)bufferViews.size();
                bufferViews.push_back({
                    {"buffer", 0},
                    {"byteOffset", idxByteOffset},
                    {"byteLength", idxByteLength},
                    {"target", 34963} // ELEMENT_ARRAY_BUFFER
                    });

                int idxAccessorIndex = (int)accessors.size();
                accessors.push_back({
                    {"bufferView", idxBVIndex},
                    {"byteOffset", 0},
                    {"componentType", 5125}, // UNSIGNED_INT
                    {"count", indexCount},
                    {"type", "SCALAR"}
                    });

                // ========= Mesh primitive & mesh =========
                json attributes;
                attributes["POSITION"] = posAccessorIndex;
                attributes["NORMAL"] = nrmAccessorIndex;
                attributes["COLOR_0"] = colAccessorIndex;
                attributes["TEXCOORD_0"] = uvAccessorIndex;

                json primitive = {
                    {"attributes", attributes},
                    {"indices", idxAccessorIndex},
                    {"material", materialIndex},
                    {"mode", 4} // TRIANGLES
                };

                json mesh = {
                    {"name", baseName},
                    {"primitives", json::array({ primitive })}
                };

                int meshIndex = (int)meshes.size();
                meshes.push_back(mesh);

                // Node for this mesh (SUBGLOB) under this GLOB
                json node = {
                    {"mesh", meshIndex},
                    {"name", baseName}
                };
                int nodeIndex = (int)nodes.size();
                nodes.push_back(node);
                nodes[globNodeIndex]["children"].push_back(nodeIndex);

                ++modelNum;
            }
        }
    }

    // ===================== Build BIN chunk (padded) =====================
    std::vector<uint8_t> binChunk = binaryBuffer;
    while (binChunk.size() % 4 != 0)
        binChunk.push_back(0x00);
    uint32_t binChunkLength = (uint32_t)binChunk.size();

    // ===================== Hook into glTF JSON =====================
    gltf["buffers"] = json::array({
        json{
            {"byteLength", binChunkLength}   // must match BIN chunk length
        }
        });
    gltf["bufferViews"] = bufferViews;
    gltf["accessors"] = accessors;
    gltf["materials"] = materials;
    if (!images.empty())   gltf["images"] = images;
    if (!textures.empty()) gltf["textures"] = textures;
    gltf["meshes"] = meshes;

    // Add lights (may modify nodes + sceneNodes + gltf.extensions)
    ExportLights(gltf, nodes, sceneNodes, zUpToYUp);

    // Finalize nodes & scene
    gltf["nodes"] = nodes;

    json scene = { {"nodes", sceneNodes}, {"name", "Scene"} };
    gltf["scenes"] = json::array({ scene });
    gltf["scene"] = 0;

    // ===================== Build JSON chunk (padded) =====================
    std::string jsonStr = gltf.dump(); // compact
    std::vector<uint8_t> jsonChunk(jsonStr.begin(), jsonStr.end());
    while (jsonChunk.size() % 4 != 0)
        jsonChunk.push_back(0x20); // spaces
    uint32_t jsonChunkLength = (uint32_t)jsonChunk.size();

    // ===================== GLB header & chunks =====================
    uint32_t magic = 0x46546C67; // "glTF"
    uint32_t version = 2;
    uint32_t totalLength = 12 + 8 + jsonChunkLength + 8 + binChunkLength;

    std::string glbPath = levelName + "/" + levelName + ".glb";
    std::ofstream out(glbPath, std::ios::binary);
    if (!out)
        return;

    auto WriteU32 = [&](uint32_t v) {
        out.write(reinterpret_cast<const char*>(&v), 4);
        };

    // GLB header
    WriteU32(magic);
    WriteU32(version);
    WriteU32(totalLength);

    // JSON chunk header + data
    WriteU32(jsonChunkLength);
    WriteU32(0x4E4F534A); // "JSON"
    out.write(reinterpret_cast<const char*>(jsonChunk.data()), jsonChunk.size());

    // BIN chunk header + data
    WriteU32(binChunkLength);
    WriteU32(0x004E4942); // "BIN\0"
    out.write(reinterpret_cast<const char*>(binChunk.data()), binChunk.size());

    out.close();
}

inline bool ComputeBarycentric(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, float& w0, float& w1, float& w2)
{
    glm::vec2 v0 = b - a;
    glm::vec2 v1 = c - a;
    glm::vec2 v2 = p - a;

    float d00 = glm::dot(v0, v0);
    float d01 = glm::dot(v0, v1);
    float d11 = glm::dot(v1, v1);
    float d20 = glm::dot(v2, v0);
    float d21 = glm::dot(v2, v1);

    float denom = d00 * d11 - d01 * d01;
    if (std::fabs(denom) < 1e-8f)
        return false;

    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    w0 = u; w1 = v; w2 = w;
    const float eps = -1e-4f;
    return (u >= eps && v >= eps && w >= eps);
}

inline glm::vec4 SampleRGBA8(const std::vector<byte>& data, int w, int h, float u, float v, const glm::vec4& defaultColor)
{
    if (data.empty() || w <= 0 || h <= 0)
        return defaultColor;

    u = glm::clamp(u, 0.0f, 1.0f);
    v = glm::clamp(v, 0.0f, 1.0f);

    int x = (int)(u * (w - 1));
    int y = (int)(v * (h - 1));   // NO flip here

    int idx = (y * w + x) * 4;
    if (idx < 0 || idx + 3 >= (int)data.size())
        return defaultColor;

    float r = data[idx + 0] / 255.0f;
    float g = data[idx + 1] / 255.0f;
    float b = data[idx + 2] / 255.0f;
    float a = data[idx + 3] / 255.0f;
    return glm::vec4(r, g, b, a);
}

MaterialCPU ProcessGlobLightingCPU(const glm::vec3& vertexColorRGB, float objectShadow, float objectMidtone, const glm::vec3& lightAccum)
{
    float dominantLight = std::max({ lightAccum.r, lightAccum.g, lightAccum.b });
    float shadowModifier = 1.0f - dominantLight;
    float clampedMidtone = glm::clamp(objectMidtone, 0.0f, shadowModifier);
    float shadowContribution =
        std::max(std::min(shadowModifier - clampedMidtone, objectShadow), 0.0f);
    float baseIntensity = glm::dot(vertexColorRGB, glm::vec3(0.3333333f));

    MaterialCPU m{};
    m.ambient = shadowContribution * baseIntensity;
    m.midtone = clampedMidtone * vertexColorRGB;
    m.light = glm::min(lightAccum, glm::vec3(1.0f)) * baseIntensity;
    return m;
}

// Evaluate ThreeWay lighting at a single point in world space
MaterialCPU EvaluateLightingAtPointCPU(const glm::vec3& worldPos, const glm::vec3& normalWorld, const glm::vec3& vertexColorRGB, float unSelfIllum)
{
    // ==== InitGlobLighting() ====
    float objectShadow  = g_psw->lsmDefault.uShadow;
    float objectMidtone = g_psw->lsmDefault.uMidtone + unSelfIllum * 0.000031f;
    glm::vec3 lightAccum(0.0f);

    // Lights in original game space (Z-up). Do NOT rotate here.
    LIGHT* plight = g_psw->dlLight.plightFirst;
    while (plight)
    {
        if (plight->lightk == LIGHTK_Direction)
        {
            // Match GLSL AddDirectionLight:
            //    float NdotL = max(dot(normalWorld, dirlight.dir.xyz), 0.0);
            //    diffuse = NdotL + NdotL^3;
            glm::vec3 Ldir = glm::normalize(glm::vec3(plight->xf.matWorld[2]));
            float NdotL = glm::max(glm::dot(normalWorld, Ldir), 0.0f);

            float diffuse = NdotL + NdotL * NdotL * NdotL;

            float shadow    = glm::max(diffuse * plight->ltfn.ruShadow    + plight->ltfn.duShadow, 0.0f);
            float midtone   = glm::max(diffuse * plight->ltfn.ruMidtone   + plight->ltfn.duMidtone, 0.0f);
            float highlight = glm::max(diffuse * plight->ltfn.ruHighlight + plight->ltfn.duHighlight, 0.0f);

            objectShadow  += shadow;
            objectMidtone += midtone;
            lightAccum    += glm::vec3(plight->rgbaColor) * highlight;
        }
        else if (plight->lightk == LIGHTK_Position)
        {
            // Match GLSL AddPositionLight:
            //  ndotl is *not* clamped before n + n^3
            glm::vec3 toLight = glm::vec3(plight->xf.posWorld) - worldPos;
            float d2 = glm::dot(toLight, toLight);
            if (d2 > 1e-12f)
            {
                float invLen = glm::inversesqrt(d2);
                glm::vec3 Lvec = toLight * invLen;

                float ndotl = glm::dot(normalWorld, Lvec);
                float diffuse = ndotl + ndotl * ndotl * ndotl;

                float att = glm::clamp(plight->agFallOff.x + plight->agFallOff.y * invLen, 0.0f, 1.0f);

                float shadow    = glm::max(diffuse * plight->ltfn.ruShadow    + plight->ltfn.duShadow, 0.0f)    * att;
                float midtone   = glm::max(diffuse * plight->ltfn.ruMidtone   + plight->ltfn.duMidtone, 0.0f)   * att;
                float highlight = glm::max(diffuse * plight->ltfn.ruHighlight + plight->ltfn.duHighlight, 0.0f) * att;

                objectShadow  += shadow;
                objectMidtone += midtone;
                lightAccum    += glm::vec3(plight->rgbaColor) * highlight;
            }
        }
        plight = plight->dleLight.plightNext;
    }

    // Same post-process as GLSL ProcessGlobLighting()
    return ProcessGlobLightingCPU(vertexColorRGB, objectShadow, objectMidtone, lightAccum);
}

glm::vec4 ComputeBakedVertexColorThreeWay(const SUBGLOB& sub, const glm::mat4& model, const glm::mat3& normalMat, const BMP* bmp, const VERTICE& v)
{
    if (!bmp || bmp->bmpWidth <= 0 || bmp->bmpHeight <= 0)
        return v.color; // fallback

    int w = bmp->bmpWidth;
    int h = bmp->bmpHeight;

    // World space
    glm::vec3 worldPos = glm::vec3(model * glm::vec4(v.pos, 1.0f));
    glm::vec3 normalWorld = glm::normalize(normalMat * v.normal);

    // Game per-vertex color used in lighting
    glm::vec3 vertexBaseColor = glm::vec3(v.color);

    // Game lighting in Z-up
    MaterialCPU m = EvaluateLightingAtPointCPU(worldPos, normalWorld, vertexBaseColor, sub.unSelfIllum);

    // Sample original textures at this vertex UV
    glm::vec2 uv = glm::fract(v.uv); // handle tiling
    float u = glm::clamp(uv.x, 0.0f, 1.0f);
    float vv = glm::clamp(uv.y, 0.0f, 1.0f);

    glm::vec4 shCol  = SampleRGBA8(bmp->shadowTexture,   w, h, u, vv, glm::vec4(0.0f));
    glm::vec4 difCol = SampleRGBA8(bmp->diffuseTexture,  w, h, u, vv, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    glm::vec4 satCol = SampleRGBA8(bmp->saturateTexture, w, h, u, vv, glm::vec4(0.0f));

    glm::vec3 shRGB  = glm::vec3(shCol);
    glm::vec3 difRGB = glm::vec3(difCol);
    glm::vec3 satRGB = glm::vec3(satCol);

    // Same mix as the pixel baker
    glm::vec3 finalRGB(0.0f);
    finalRGB += shRGB  * m.ambient;
    finalRGB += difRGB * m.midtone;
    finalRGB += satRGB * m.light;
    finalRGB = glm::clamp(finalRGB, 0.0f, 1.0f);

    glm::vec3 factor(1.0f);
    const float eps = 1.0f / 255.0f;

    for (int c = 0; c < 3; ++c)
    {
        float base = difRGB[c];
        float f = finalRGB[c];

        if (base > eps)
            factor[c] = f / base;
        else
            factor[c] = f;
    }

    factor = glm::clamp(factor, glm::vec3(0.0f), glm::vec3(4.0f));

    float alpha = glm::clamp(v.color.a, 0.0f, 1.0f);

    return glm::vec4(factor, alpha);
}

void ExportLights(json& gltf, std::vector<json>& nodes, std::vector<int>& sceneNodes, const glm::mat4& zUpToYUp)
{
    // --- 1) Mark extension as used/required ---
    if (!gltf.contains("extensionsUsed") || !gltf["extensionsUsed"].is_array())
        gltf["extensionsUsed"] = json::array();
    auto& extUsed = gltf["extensionsUsed"];
    if (std::find(extUsed.begin(), extUsed.end(), "KHR_lights_punctual") == extUsed.end())
        extUsed.push_back("KHR_lights_punctual");

    if (!gltf.contains("extensionsRequired") || !gltf["extensionsRequired"].is_array())
        gltf["extensionsRequired"] = json::array();
    auto& extReq = gltf["extensionsRequired"];
    if (std::find(extReq.begin(), extReq.end(), "KHR_lights_punctual") == extReq.end())
        extReq.push_back("KHR_lights_punctual");

    if (!gltf.contains("extensions") || !gltf["extensions"].is_object())
        gltf["extensions"] = json::object();

    json& rootExt = gltf["extensions"];
    if (!rootExt.contains("KHR_lights_punctual") ||
        !rootExt["KHR_lights_punctual"].is_object())
    {
        rootExt["KHR_lights_punctual"] = json::object();
    }

    json& lightsExt = rootExt["KHR_lights_punctual"];
    if (!lightsExt.contains("lights") || !lightsExt["lights"].is_array())
        lightsExt["lights"] = json::array();

    json& lightsArray = lightsExt["lights"];

    // --- 2) DIRECTIONAL LIGHT (your existing code) ---
    {
        LIGHT* plight = g_psw->dlLight.plightFirst;
        LIGHT* pDir = nullptr;

        while (plight != nullptr) {
            if (plight->lightk == LIGHTK_Direction) {
                pDir = plight;
                break;
            }
            plight = plight->dleLight.plightNext;
        }

        if (pDir) {
            glm::vec3 dirZup = glm::vec3(pDir->xf.matWorld[2]);
            glm::vec3 dirYup = glm::mat3(zUpToYUp) * dirZup;
            dirYup = glm::normalize(dirYup);

            glm::vec3 col = glm::vec3(pDir->rgbaColor);

            json lightDef = {
                {"name", "DirectionalLight_0"},
                {"type", "directional"},
                {"color", { col.r, col.g, col.b }},
                {"intensity", 5.0f}
            };

            int lightIndex = (int)lightsArray.size();
            lightsArray.push_back(lightDef);

            glm::vec3 fromDir(0.0f, 0.0f, -1.0f);
            glm::quat q = RotationBetweenVectors(fromDir, dirYup);

            json lightNode;
            lightNode["name"] = "DirectionalLightNode_0";
            lightNode["rotation"] = { pDir->xf.matWorld[2][0], pDir->xf.matWorld[2][1], pDir->xf.matWorld[2][2], 1.0 };
            lightNode["extensions"] = {
                { "KHR_lights_punctual", { { "light", lightIndex } } }
            };

            int nodeIndex = (int)nodes.size();
            nodes.push_back(lightNode);
            sceneNodes.push_back(nodeIndex);
        }
    }

    // --- 3) POINT LIGHTS WITH ATTENUATION ---
    {
        LIGHT* plight = g_psw->dlLight.plightFirst;
        int pointIdx = 0;

        while (plight != nullptr)
        {
            if (plight->lightk == LIGHTK_Position)
            {
                // Color
                glm::vec3 col = glm::vec3(plight->rgbaColor);

                // Position: Z-up -> Y-up
                glm::vec3 posZ = plight->xf.posWorld;
                glm::vec3 posY = glm::vec3(zUpToYUp * glm::vec4(posZ, 1.0f));

                // Range for attenuation
                float range = plight->lmFallOffS.gMax;        // your max influence distance
                if (range <= 0.0f)
                    range = 10.0f;                // fallback

                // Base intensity (tune if too bright/dim in Blender)
                float baseIntensity = 50.0f;

                json pointLight = {
                    {"name", "PointLight_" + std::to_string(pointIdx)},
                    {"type", "point"},
                    {"color", { col.r, col.g, col.b }},
                    {"intensity", baseIntensity},
                    {"range", range}
                };

                int lightIndex = (int)lightsArray.size();
                lightsArray.push_back(pointLight);

                // Node for this point light
                json node;
                node["name"] = "PointLightNode_" + std::to_string(pointIdx);
                node["translation"] = { posY.x, posY.y, posY.z };
                node["extensions"] = {
                    { "KHR_lights_punctual", { { "light", lightIndex } } }
                };

                int nodeIndex = (int)nodes.size();
                nodes.push_back(node);
                sceneNodes.push_back(nodeIndex);

                ++pointIdx;
            }

            plight = plight->dleLight.plightNext;
        }
    }
}

void ExportSwLightsBaked()
{
    std::filesystem::create_directory(levelName);

    std::vector<uint8_t> binaryBuffer;

    json gltf;
    std::vector<json> bufferViews;
    std::vector<json> accessors;
    std::vector<json> materials;
    std::vector<json> images;
    std::vector<json> textures;
    std::vector<json> meshes;
    std::vector<json> nodes;
    std::vector<int>  sceneNodes;

    gltf["asset"] = {
        {"version", "2.0"},
        {"generator", "ExportSwGLB"}
    };

    auto EnsureUnlitExtension = [&](json& mat)
        {
            if (!gltf.contains("extensionsUsed") || !gltf["extensionsUsed"].is_array())
                gltf["extensionsUsed"] = json::array();

            auto& extUsed = gltf["extensionsUsed"];
            if (std::find(extUsed.begin(), extUsed.end(), "KHR_materials_unlit") == extUsed.end())
                extUsed.push_back("KHR_materials_unlit");

            mat["extensions"] = {
                {"KHR_materials_unlit", json::object()}
            };
        };

    // Add Z-up -> Y-up rotation
    glm::mat4 zUpToYUp = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(-90.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    // Default material
    int defaultMaterialIndex = 0;
    {
        json mat = {
            {"name", "DefaultMaterial"},
            {"pbrMetallicRoughness", {
                {"baseColorFactor", {1.0,1.0,1.0,1.0}},
                {"metallicFactor", 0.0},
                {"roughnessFactor", 1.0}
            }},
            {"alphaMode", "OPAQUE"}
        };
        EnsureUnlitExtension(mat);
        materials.push_back(mat);
    }

    std::unordered_map<int, int> shdToMaterialIndex;

    // PASS 0: Determine which SHDs need blending
    struct ShdUsageInfo { bool wantsAlphaBlend = false; };
    std::unordered_map<int, ShdUsageInfo> shdUsage;

    for (auto* alo : allSWAloObjs)
    {
        if (!alo) continue;

        for (auto& glob : alo->globset.aglob)
        {
            for (auto& sub : glob.asubglob)
            {
                int shdID = sub.shdID;
                if (shdID < 0 || shdID >= (int)g_ashd.size()) continue;

                SHD& shd = g_ashd[shdID];
                bool alpha = false;

                if (glob.rp == RP_Cutout || glob.rp == RP_CutoutAfterProjVolume || glob.rp == RP_Translucent)
                {
                    alpha = true;
                }

                if (glob.rp == RP_Background && shd.grfshd == 2)
                    alpha = true;

                if (alpha)
                    shdUsage[shdID].wantsAlphaBlend = true;
            }
        }
    }

    // PASS 1: Build materials
    for (int shdID = 0; shdID < (int)g_ashd.size(); shdID++)
    {
        SHD& shd = g_ashd[shdID];
        if (shd.atex.empty() || shd.atex[0].abmp.empty() || !shd.atex[0].abmp[0])
            continue;

        BMP* bmp = shd.atex[0].abmp[0];
        if (!bmp || bmp->diffuseTexture.empty())
        {
            shdToMaterialIndex[shdID] = defaultMaterialIndex;
            continue;
        }

        PngBuffer png;
        stbi_write_png_to_func(
            PngWriteCallback,
            &png,
            bmp->bmpWidth,
            bmp->bmpHeight,
            4,
            bmp->diffuseTexture.data(),
            bmp->bmpWidth * 4
        );

        if (png.data.empty())
        {
            shdToMaterialIndex[shdID] = defaultMaterialIndex;
            continue;
        }

        AlignTo4(binaryBuffer);
        size_t pngOffset = binaryBuffer.size();
        binaryBuffer.insert(binaryBuffer.end(), png.data.begin(), png.data.end());

        int viewIdx = (int)bufferViews.size();
        bufferViews.push_back({
            {"buffer",0},
            {"byteOffset",pngOffset},
            {"byteLength",png.data.size()}
            });

        int imgIdx = (int)images.size();
        images.push_back({
            {"bufferView",viewIdx},
            {"mimeType","image/png"}
            });

        int texIdx = (int)textures.size();
        textures.push_back({ {"source",imgIdx} });

        bool blend = shdUsage[shdID].wantsAlphaBlend;

        json mat = {
            {"name","Material_Shader_" + std::to_string(shdID)},
            {"pbrMetallicRoughness",{
                {"baseColorTexture",{{"index",texIdx}}},
                {"metallicFactor",0.0},
                {"roughnessFactor",1.0}
            }}
        };

        EnsureUnlitExtension(mat);
        mat["alphaMode"] = blend ? "BLEND" : "OPAQUE";
        if (blend) mat["doubleSided"] = true;

        int matIndex = (int)materials.size();
        materials.push_back(mat);
        shdToMaterialIndex[shdID] = matIndex;
    }

    // PASS 2: GEOMETRY
    uint32_t aloIndex = 0;

    for (auto* alo : allSWAloObjs)
    {
        if (!alo)
        {
            aloIndex++;
            continue;
        }

        // One GLOBSET per ALO
        int globsetNodeIndex = (int)nodes.size();
        {
            json globsetNode = {
                {"name", "GLOBSET_" + std::to_string(aloIndex)},
                {"children", json::array()}
            };
            nodes.push_back(globsetNode);
            sceneNodes.push_back(globsetNodeIndex);
        }

        uint32_t globIndex = 0;

        for (auto& glob : alo->globset.aglob)
        {
            glm::mat4 modelZ(1.0f);
            modelZ = alo->xf.matWorld;
            modelZ[3][0] = alo->xf.posWorld.x;
            modelZ[3][1] = alo->xf.posWorld.y;
            modelZ[3][2] = alo->xf.posWorld.z;

            if (glob.pdmat)
                modelZ = modelZ * *glob.pdmat;

            glm::mat4 modelY = zUpToYUp * modelZ;

            glm::mat3 normalMatZ = glm::mat3(glm::transpose(glm::inverse(modelZ)));
            glm::mat3 normalMatY = glm::mat3(glm::transpose(glm::inverse(modelY)));

            // GLOB node under this ALO's GLOBSET
            int globNodeIndex = (int)nodes.size();
            {
                json globNode = {
                    {"name", "GLOB_" + std::to_string(aloIndex) + "_" + std::to_string(globIndex)},
                    {"children", json::array()}
                };
                nodes.push_back(globNode);
                nodes[globsetNodeIndex]["children"].push_back(globNodeIndex);
            }

            uint32_t subIndex = 0;

            for (auto& sub : glob.asubglob)
            {
                size_t vc = sub.vertices.size();
                size_t tc = sub.indices.size();
                if (vc == 0 || tc == 0)
                {
                    subIndex++;
                    continue;
                }

                int matIndex = defaultMaterialIndex;
                if (sub.shdID >= 0 && sub.shdID < (int)g_ashd.size())
                {
                    auto it = shdToMaterialIndex.find(sub.shdID);
                    if (it != shdToMaterialIndex.end())
                        matIndex = it->second;
                }

                BMP* bmp = nullptr;
                if (sub.pshd && !sub.pshd->atex.empty() && !sub.pshd->atex[0].abmp.empty())
                    bmp = sub.pshd->atex[0].abmp[0];

                bool isThreeWay = (sub.pshd && sub.pshd->shdk == SHDK_ThreeWay && bmp != nullptr);

                // POSITIONS (Y-up)
                AlignTo4(binaryBuffer);
                size_t posOffset = binaryBuffer.size();
                for (auto& v : sub.vertices)
                {
                    glm::vec3 p = glm::vec3(modelY * glm::vec4(v.pos, 1.0f));
                    AppendFloat(binaryBuffer, p.x);
                    AppendFloat(binaryBuffer, p.y);
                    AppendFloat(binaryBuffer, p.z);
                }
                size_t posLen = binaryBuffer.size() - posOffset;

                int posView = (int)bufferViews.size();
                bufferViews.push_back({
                    {"buffer",0},
                    {"byteOffset",posOffset},
                    {"byteLength",posLen},
                    {"target",34962}
                    });

                int posAccessor = (int)accessors.size();
                accessors.push_back({
                    {"bufferView",posView},
                    {"byteOffset",0},
                    {"componentType",5126},
                    {"count",vc},
                    {"type","VEC3"}
                    });

                // NORMALS (Y-up)
                AlignTo4(binaryBuffer);
                size_t nrmOffset = binaryBuffer.size();
                for (auto& v : sub.vertices)
                {
                    glm::vec3 n = glm::normalize(normalMatY * v.normal);
                    AppendFloat(binaryBuffer, n.x);
                    AppendFloat(binaryBuffer, n.y);
                    AppendFloat(binaryBuffer, n.z);
                }
                size_t nrmLen = binaryBuffer.size() - nrmOffset;

                int nrmView = (int)bufferViews.size();
                bufferViews.push_back({
                    {"buffer",0},
                    {"byteOffset",nrmOffset},
                    {"byteLength",nrmLen},
                    {"target",34962}
                    });

                int nrmAccessor = (int)accessors.size();
                accessors.push_back({
                    {"bufferView",nrmView},
                    {"byteOffset",0},
                    {"componentType",5126},
                    {"count",vc},
                    {"type","VEC3"}
                    });

                // COLORS (baked in Z-up)
                AlignTo4(binaryBuffer);
                size_t colOffset = binaryBuffer.size();

                for (auto& v : sub.vertices)
                {
                    glm::vec4 col = v.color;

                    if (isThreeWay)
                    {
                        col = ComputeBakedVertexColorThreeWay(
                            sub,
                            modelZ,
                            normalMatZ,
                            bmp,
                            v
                        );
                    }

                    AppendFloat(binaryBuffer, col.r);
                    AppendFloat(binaryBuffer, col.g);
                    AppendFloat(binaryBuffer, col.b);
                    AppendFloat(binaryBuffer, col.a);
                }

                size_t colLen = binaryBuffer.size() - colOffset;

                int colView = (int)bufferViews.size();
                bufferViews.push_back({
                    {"buffer",0},
                    {"byteOffset",colOffset},
                    {"byteLength",colLen},
                    {"target",34962}
                    });

                int colAccessor = (int)accessors.size();
                accessors.push_back({
                    {"bufferView",colView},
                    {"byteOffset",0},
                    {"componentType",5126},
                    {"count",vc},
                    {"type","VEC4"}
                    });

                // UVs
                AlignTo4(binaryBuffer);
                size_t uvOffset = binaryBuffer.size();
                for (auto& v : sub.vertices)
                {
                    AppendFloat(binaryBuffer, v.uv.x);
                    AppendFloat(binaryBuffer, v.uv.y);
                }
                size_t uvLen = binaryBuffer.size() - uvOffset;

                int uvView = (int)bufferViews.size();
                bufferViews.push_back({
                    {"buffer",0},
                    {"byteOffset",uvOffset},
                    {"byteLength",uvLen},
                    {"target",34962}
                    });

                int uvAccessor = (int)accessors.size();
                accessors.push_back({
                    {"bufferView",uvView},
                    {"byteOffset",0},
                    {"componentType",5126},
                    {"count",vc},
                    {"type","VEC2"}
                    });

                // INDICES
                AlignTo4(binaryBuffer);
                size_t idxOffset = binaryBuffer.size();
                uint32_t idxCount = (uint32_t)(tc * 3);
                for (auto& tri : sub.indices)
                {
                    AppendUInt32(binaryBuffer, tri.v1);
                    AppendUInt32(binaryBuffer, tri.v2);
                    AppendUInt32(binaryBuffer, tri.v3);
                }
                size_t idxLen = binaryBuffer.size() - idxOffset;

                int idxView = (int)bufferViews.size();
                bufferViews.push_back({
                    {"buffer",0},
                    {"byteOffset",idxOffset},
                    {"byteLength",idxLen},
                    {"target",34963}
                    });

                int idxAccessor = (int)accessors.size();
                accessors.push_back({
                    {"bufferView",idxView},
                    {"byteOffset",0},
                    {"componentType",5125},
                    {"count",idxCount},
                    {"type","SCALAR"}
                    });

                // Build mesh & SUBGLOB node
                json attributes;
                attributes["POSITION"] = posAccessor;
                attributes["NORMAL"] = nrmAccessor;
                attributes["TEXCOORD_0"] = uvAccessor;
                attributes["COLOR_0"] = colAccessor;

                json primitive = {
                    {"attributes",attributes},
                    {"indices",idxAccessor},
                    {"material",matIndex},
                    {"mode",4}
                };

                std::string baseName =
                    "SUBGLOB_" + std::to_string(aloIndex) + "_" +
                    std::to_string(globIndex) + "_" +
                    std::to_string(subIndex);

                json mesh = {
                    {"name",baseName},
                    {"primitives", json::array({primitive})}
                };

                int meshIndex = (int)meshes.size();
                meshes.push_back(mesh);

                json node = {
                    {"mesh",meshIndex},
                    {"name",baseName}
                };

                int nodeIndex = (int)nodes.size();
                nodes.push_back(node);

                // Attach SUBGLOB node under this GLOB node
                nodes[globNodeIndex]["children"].push_back(nodeIndex);

                subIndex++;
            }

            globIndex++;
        }

        aloIndex++;
    }

    // Build GLB
    std::vector<uint8_t> binChunk = binaryBuffer;
    while (binChunk.size() % 4 != 0) binChunk.push_back(0);

    gltf["buffers"] = json::array({
        json{{"byteLength",(uint32_t)binChunk.size()}}
        });

    gltf["bufferViews"] = bufferViews;
    gltf["accessors"] = accessors;
    gltf["materials"] = materials;
    if (!images.empty()) gltf["images"] = images;
    if (!textures.empty()) gltf["textures"] = textures;
    gltf["meshes"] = meshes;
    gltf["nodes"] = nodes;

    gltf["scenes"] = json::array({
        json{{"name","Scene"},{"nodes",sceneNodes}}
        });
    gltf["scene"] = 0;

    std::string jsonStr = gltf.dump();
    std::vector<uint8_t> jsonChunk(jsonStr.begin(), jsonStr.end());
    while (jsonChunk.size() % 4 != 0) jsonChunk.push_back(0x20);

    uint32_t totalLength =
        12 +                       // header
        8 + (uint32_t)jsonChunk.size() +     // JSON chunk header + data
        8 + (uint32_t)binChunk.size();       // BIN chunk header + data

    std::string outPath = levelName + "/" + levelName + ".glb";
    std::ofstream out(outPath, std::ios::binary);
    if (!out) return;

    auto WriteU32 = [&](uint32_t v)
        {
            out.write((const char*)&v, 4);
        };

    WriteU32(0x46546C67);
    WriteU32(2);
    WriteU32(totalLength);

    WriteU32((uint32_t)jsonChunk.size());
    WriteU32(0x4E4F534A);
    out.write((const char*)jsonChunk.data(), jsonChunk.size());

    WriteU32((uint32_t)binChunk.size());
    WriteU32(0x004E4942);
    out.write((const char*)binChunk.data(), binChunk.size());
}

void ExportTextures()
{
    std::string textureDir = levelName + "/Textures";
    std::filesystem::create_directories(textureDir);

    for (size_t i = 0; i < g_ashd.size(); ++i)
    {
        SHD& shd = g_ashd[i];
        BMP* bmp = shd.atex[0].abmp[0];

        if (bmp != nullptr)
        {
            if (bmp->shadowTexture.size() > 0)
            {
                char filenameShadow[256];
                snprintf(filenameShadow, sizeof(filenameShadow), "%s/shader_%03zu_shadow.png", textureDir.c_str(), i);
                stbi_flip_vertically_on_write(1);
                stbi_write_png(filenameShadow, bmp->bmpWidth, bmp->bmpHeight, 4, bmp->shadowTexture.data(), bmp->bmpWidth * 4);
                stbi_flip_vertically_on_write(0);
            }

            if (bmp->diffuseTexture.size() > 0)
            {
                char filenameDiffuse[256];
                snprintf(filenameDiffuse, sizeof(filenameDiffuse), "%s/shader_%03zu_diffuse.png", textureDir.c_str(), i);
                stbi_flip_vertically_on_write(1);
                stbi_write_png(filenameDiffuse, bmp->bmpWidth, bmp->bmpHeight, 4, bmp->diffuseTexture.data(), bmp->bmpWidth * 4);
                stbi_flip_vertically_on_write(0);
            }

            if (bmp->saturateTexture.size() > 0)
            {
                char filenameSaturate[256];
                snprintf(filenameSaturate, sizeof(filenameSaturate), "%s/shader_%03zu_saturate.png", textureDir.c_str(), i);
                stbi_flip_vertically_on_write(1);
                stbi_write_png(filenameSaturate, bmp->bmpWidth, bmp->bmpHeight, 4, bmp->saturateTexture.data(), bmp->bmpWidth * 4);
                stbi_flip_vertically_on_write(0);
            }
        }
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
float baseRenderDistance = 0.0;