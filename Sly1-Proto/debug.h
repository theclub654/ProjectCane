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
extern inline bool fRenderModels = true;
extern inline bool fRenderCollision = false;
extern inline bool fMouseHoverGui = false;
void DeleteWorld(SW* psw);

void RenderMenuGui(SW* psw);
void ExportSw();