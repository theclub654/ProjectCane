#pragma once
#include <filesystem>
#include <nlohmann/json.hpp>
#include <glm/gtx/quaternion.hpp>
#include "transition.h" 
#include "light.h"

using json = nlohmann::json;

struct MaterialCPU
{
    float ambient;
    glm::vec3 midtone;
    glm::vec3 light;
};

struct PngBuffer {
    std::vector<uint8_t> data;
};

void RenderMenuGui(SW* psw);
static void AppendFloat(std::vector<uint8_t>& buf, float v);
static void AppendUInt32(std::vector<uint8_t>& buf, uint32_t v);
static void AlignTo4(std::vector<uint8_t>& buf);
static void PngWriteCallback(void* context, void* data, int size);
static glm::quat RotationBetweenVectors(const glm::vec3& from, const glm::vec3& to);
void ExportSw();
inline bool ComputeBarycentric(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, float& w0, float& w1, float& w2);
MaterialCPU EvaluateLightingAtPointCPU(const glm::vec3& worldPos, const glm::vec3& normalWorld, const glm::vec3& vertexColorRGB, float unSelfIllum);
static MaterialCPU ProcessGlobLightingCPU(const glm::vec3& vertexColorRGB, float objectShadow, float objectMidtone, const glm::vec3& lightAccum);
void ExportLights(json& gltf, std::vector<json>& nodes, std::vector<int>& sceneNodes, const glm::mat4& zUpToYUp);
void ExportSwLightsBaked();
void ExportTextures();

extern std::string file;
extern std::string levelName;
extern std::string filePath;
extern ImGuiFileDialog instance_a;
extern bool g_fDisableInput;
extern float baseRenderDistance;