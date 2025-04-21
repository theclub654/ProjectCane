#pragma once
#include <filesystem>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/ImGuiFileDialog.h"
#include "transition.h"

extern std::string file;
extern std::string levelName;
extern std::vector <ALO*> allSWAloObjs;
static std::string filePath;
static ImGuiFileDialog instance_a;
extern inline bool g_fRenderModels = true;
extern inline bool g_fRenderCelBorders = true;
extern inline int  g_fogType = 1;
extern inline bool g_fDisableInput = false;
extern inline bool g_fRenderCollision = false; // g_fShowCameraEdges

void DeleteWorld(SW* psw);

void RenderMenuGui(SW* psw);
void ExportSw();